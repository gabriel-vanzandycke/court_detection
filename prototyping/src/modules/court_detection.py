
from calib3d import Calib
import cv2
import distinctipy
import numpy as np
import sknetwork.clustering

from src.utils.court_definition import Court
from src.utils.utils import display_image, LineSegment, draw_line, EPS, closest_point, distance_to_line, lines_intersection
from src.cv.image_processing import SegmentsDetection, Skeletonize, FilterConnectedComponentsBySize


def display_lines(data):
    data['debug_image'] = data['original'].copy()
    colors = distinctipy.get_colors(len(data['lines']))
    for i, (line, color) in enumerate(zip(data['lines'], colors)):
        color = list(map(lambda x: int(x*255), color))
        draw_line(data['debug_image'], line, color, 3, markersize=10, label=str(i))
    return data

class CourtDetector:
    def __init__(self, court_type="ITF", debug=False):
        self.court = Court(court_type)
        self.processors = [
            lambda data: data.update({'original': np.dstack([data['image']]*3)}),
            lambda data: data.update({'debug': True}) if debug else None,
            Skeletonize(),
            lambda data: display_image(data['image'], "after skeletonization") if debug else None,
            FilterConnectedComponentsBySize(lambda sizes: sizes < 50),
            lambda data: display_image(data['image'], "after small components filtering") if debug else None,
            SegmentsDetection(10, min_length=100, max_gap=100, angle_step=1, distance_step=1),
            ClusterDetectedSegments(),
            display_lines if debug else None,
            LabelLines(),
            FindProjection(self.court),
        ]

    def __call__(self, input_image):
        # lines detection
        data = {'image': input_image.copy(), 'width': input_image.shape[1], 'height': input_image.shape[0]}
        for processor in self.processors:
            if processor is not None:
                processor(data)
                if data.get("debug_image", None) is not None:
                    display_image(data['debug_image'], "after {}".format(processor.__class__.__name__))
                    del data["debug_image"]
        return data


class ClusterDetectedSegments:
    """ Cluster detected segments into longer segments using a clustering algorithm
    """
    def __init__(self, radius_threshold=50, angles_threshold=5):
        self.radius_threshold = radius_threshold
        self.angles_threshold = angles_threshold
    def __call__(self, data):
        segments = data['segments']
        canvas = data['debug_image'] = data['original'].copy() if data.get('debug', False) else None

        radius = np.array([s.rho for s in segments]).reshape((-1, 1))
        angles = np.array([s.theta for s in segments]).reshape((-1, 1))
        adjacency = (np.abs(radius.T - radius) < self.radius_threshold) & ((np.abs(angles.T - angles) % np.pi)*180/np.pi < self.angles_threshold)
        labels = sknetwork.clustering.PropagationClustering().fit_transform(adjacency)
        indices = np.unique(labels)

        if canvas is not None:
            colors = distinctipy.get_colors(len(indices))
            for i, color in zip(indices, colors):
                color = list(map(lambda x: x*255, color))
                for segment in np.array(segments)[np.where(labels==i)[0]]:
                    draw_line(canvas, segment, color, 3, markersize=10, label=f"{i}: |{int(segment.rho)}|, {int(segment.theta*180/np.pi)}deg")

        def merge(segments):
            # LineSegment in the form `mx + py = 1` to support vertical segments.
            # Solving linear model A @ [[m],
            #                           [p]] = b
            A = np.array([[[l.x1, l.y1], [l.x2, l.y2]] for l in segments]).reshape((-1, 2))
            b = np.ones((A.shape[0], 1))
            m, p = (np.linalg.inv(A.T@A)@A.T@b).flatten()
            theta = np.arctan2(p, m)
            if np.abs(p) < EPS: # Vertical segment:
                x1, y1 = A[np.argmin(A[:, 1])] # use max y
                x2, y2 = A[np.argmax(A[:, 1])] # use min y
                rho = np.cos(theta)/m
            else:
                x1, y1 = A[np.argmin(A[:, 0])] # use max x
                x2, y2 = A[np.argmax(A[:, 0])] # use min x
                rho = np.sin(theta)/p
            x1, y1 = closest_point(rho, theta, x1, y1).flatten()
            x2, y2 = closest_point(rho, theta, x2, y2).flatten()
            return LineSegment(x1, y1, x2, y2)

        data['lines'] = [merge([segments[i] for i in np.where(labels==i)[0]]) for i in indices]


class LabelLines:
    """ From a set of lines detected in the image, label lines relevant for homography estimation (left and right
        sidelines, baseline, and service line.
    """
    def __init__(self, distance_threshold=20):
        self.distance_threshold = distance_threshold
    def __call__(self, data):
        horizontal_lines = [l for l in data['lines'] if l.orientation == 'horizontal']
        serveline = sorted(horizontal_lines, key=lambda l: l.length)[0]
        baseline = sorted([l for l in horizontal_lines if l.y_intercept > serveline.y_intercept], key=lambda l: l.y_intercept)[0]

        predicate = lambda l: l.orientation == 'vertical' and \
            ( distance_to_line(l.rho, l.theta, serveline.x1, serveline.y1) < self.distance_threshold or \
              distance_to_line(l.rho, l.theta, serveline.x2, serveline.y2) < self.distance_threshold )
        single_sidelines = [l for l in data['lines'] if predicate(l)]

        predicate = lambda l: l.orientation == 'vertical' and \
            ( np.linalg.norm(np.array([l.x1, l.y1]) - np.array([serveline.x1+serveline.x2, serveline.y1+serveline.y2])/2) < self.distance_threshold or \
              np.linalg.norm(np.array([l.x2, l.y2]) - np.array([serveline.x1+serveline.x2, serveline.y1+serveline.y2])/2) < self.distance_threshold )
        centerline = [l for l in data['lines'] if predicate(l)][0]

        data['debug_image'] = data['original'].copy() if data.get('debug', False) else None
        if data['debug_image'] is not None:
            for l in single_sidelines:
                draw_line(data['debug_image'], l, (0, 0, 255), 2, label='single sideline')
            draw_line(data['debug_image'], serveline, (0, 255, 0), 2, label='serveline')
            draw_line(data['debug_image'], baseline, (0, 255, 0), 2, label='baseline')
            draw_line(data['debug_image'], centerline, (255, 0, 0), 2, label='centerline')

        data['lines'] = [serveline, baseline, centerline, *single_sidelines]
        data['serveline'] = serveline
        data['baseline'] = baseline
        data['centerline'] = centerline
        data['right_sidelines'] = max(single_sidelines, key=lambda l: l.x_intercept)
        data['left_sidelines'] = min(single_sidelines, key=lambda l: l.x_intercept)
        return data


class FindProjection:
    def __init__(self, court):
        self.court = court
    def __call__(self, data):
        serveline = data['serveline']
        baseline = data['baseline']
        centerline = data['centerline']
        right_sideline = data['right_sidelines']
        left_sideline = data['left_sidelines']
        width = data['width']
        height = data['height']

        image_points = np.array([
            lines_intersection(serveline.rho, serveline.theta, left_sideline.rho, left_sideline.theta),   # A
            lines_intersection(serveline.rho, serveline.theta, right_sideline.rho, right_sideline.theta), # B
            lines_intersection(baseline.rho, baseline.theta, left_sideline.rho, left_sideline.theta),     # C
            lines_intersection(baseline.rho, baseline.theta, right_sideline.rho, right_sideline.theta),   # D
            lines_intersection(serveline.rho, serveline.theta, centerline.rho, centerline.theta),         # E
        ]).astype(np.float32).squeeze()

        world_points = self.court.keypoints.astype(np.float32)

        _, K, kc, r, t = cv2.calibrateCamera([world_points], [image_points], (width, height), cameraMatrix=None, distCoeffs=None,
            flags = cv2.CALIB_FIX_ASPECT_RATIO | cv2.CALIB_ZERO_TANGENT_DIST | cv2.CALIB_FIX_K1 | cv2.CALIB_FIX_K2 | cv2.CALIB_FIX_K3 | cv2.CALIB_FIX_K4 | cv2.CALIB_FIX_K5 | cv2.CALIB_FIX_K6)
        T = t[0]
        R = cv2.Rodrigues(r[0])[0]

        data['calib'] = Calib(width=width, height=height, T=T, R=R, K=K)


class LinesFromPoints:
    """ LineSegment detection from points
        Args:
            radius_threshold (float): lines similarity distance threshold
            angles_threshold (float): lines similarity angle threshold
            min_aligned (int): minimum number of points aligned to consider a line
        # FIXME: There could be a bug with angle calculation
    """
    def __init__(self, radius_threshold=2, angles_threshold=.002, min_aligned=3) -> None:
        self.radius_threshold = radius_threshold
        self.angles_threshold = angles_threshold
        self.min_aligned = min_aligned

    def __call__(self, data):
        points = data['points']

        # Build each line from pair of points
        radius = np.zeros((len(points), len(points), 1))
        angles = np.zeros((len(points), len(points), 1))
        for i, (x1, y1) in enumerate(points):
            for j, (x2, y2) in enumerate(points):
                radius[i,j] = np.abs((x2-x1)*y1 - x1*(y2-y1))/np.sqrt((x2-x1)**2 + (y2-y1)**2) if x1 != x2 and y1 != y2 else np.nan
                angles[i,j] = np.arctan2(y2-y1, x2-x1)

        # Cluster lines with similar radius and angles
        radius = radius.reshape((-1, 1))
        angles = angles.reshape((-1, 1))
        adjacency = (np.abs(radius.T - radius) < self.radius_threshold) & (np.abs(angles.T - angles) < self.angles_threshold) & (np.isfinite(radius))
        labels = sknetwork.clustering.PropagationClustering().fit_transform(adjacency)

        data['tmp'] = np.dstack([data['original_image'], data['original_image'], data['original_image']])
        # Select lines with enough points
        lines = []
        for label in np.unique(labels):
            indices = labels==label
            if sum(indices) >= self.min_aligned:
                r = np.mean(radius[indices])
                a = np.mean(angles[indices])
                indices = np.unravel_index(np.where(indices)[0], (len(points), len(points)))
                pts = np.unique(np.array([[points[i1], points[i2]] for i1, i2 in zip(*indices)]).reshape((-1, 2)), axis=0)
                for pt in pts:
                    cv2.circle(data['tmp'], tuple(pt), 5, (0, 0, 255), 2)
                xs = pts[:, 0]
                p1 = closest_point(r, a, *pts[np.argmax(xs)])
                p2 = closest_point(r, a, *pts[np.argmin(xs)])
                cv2.circle(data['tmp'], tuple(np.intp(p1.flatten())), 5, (255, 0, 0), 2)
                cv2.circle(data['tmp'], tuple(np.intp(p2.flatten())), 5, (255, 0, 0), 2)
                x, y = r*np.array([[np.cos(a)], [np.sin(a)]]).flatten()
                cv2.circle(data['tmp'], (int(x), int(y)), 5, (0, 255, 0), 2)
                lines.append(LineSegment(*p1, *p2))
        data['lines'] = lines
        return data
