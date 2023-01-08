import cv2
import numpy as np
from src.utils.utils import LineSegment, draw_line
import distinctipy

class FilterConnectedComponentsBySize:
    """ Filter connected components by size
        Args:
            predicate (callable): A function that takes a list of connected
                                  component sizes and returns a boolean array
    """
    def __init__(self, predicate=lambda sizes: sizes < 50) -> None:
        self.predicate = predicate
    def __call__(self, data):
        _, labels, stats, _ = cv2.connectedComponentsWithStats(data['image'])
        indices = np.where(self.predicate(stats[:, -1]))[0] # Filter connected components by size
        for i in indices[1:]: # Ignore background connected component
            data['image'][labels == i] = 0
        return data

class Skeletonize:
    """ Skeletonize the image
    """
    def __init__(self, thinning_type=cv2.ximgproc.THINNING_ZHANGSUEN) -> None:
        self.type = thinning_type
    def __call__(self, data):
        data['image'] = cv2.ximgproc.thinning(data['image'], thinningType=self.type)
        return data

class Blur:
    """ Blur the image
    """
    def __init__(self, ksize=5) -> None:
        self.kernel = (ksize, ksize)
    def __call__(self, data):
        data['image'] = cv2.GaussianBlur(data['image'], self.kernel, 0)
        return data

class Openning:
    """ Openning the image
    """
    def __init__(self, kernel_size=(5, 5)) -> None:
        self.kernel_size = kernel_size
    def __call__(self, data):
        kernel = np.ones(self.kernel_size, np.uint8)
        data['image'] = cv2.morphologyEx(data['image'], cv2.MORPH_OPEN, kernel)
        return data

class CannyEdgeDetection:
    """ Canny edge detection
    """
    def __init__(self, hysteresis_thresholds=(50, 150), aperture_size=3) -> None:
        self.hysteresis_thresholds = hysteresis_thresholds
        self.aperture_size = aperture_size
    def __call__(self, data):
        t1, t2 = self.hysteresis_thresholds
        data['image'] = cv2.Canny(data['image'], t1, t2, self.aperture_size)
        return data

class LaplacianEdgeDetection:
    """ Laplacian edge detection
    """
    def __init__(self, ksize=3) -> None:
        self.ksize = ksize
    def __call__(self, data):
        ddepth = {
            np.uint8: cv2.CV_16S,
        }[data['image'].dtype] # Avoid overflow
        data['image'] = cv2.Laplacian(data['image'], ddepth, self.ksize)
        return data

class HarrisCornerDetection:
    """ Harris corner detection
        (from https://docs.opencv.org/3.4/dc/d0d/tutorial_py_features_harris.html)
    """
    def __init__(self, block_size=10, ksize=9, k=0.2, threshold=0.01) -> None:
        self.block_size = block_size
        self.ksize = ksize
        self.k = k
        self.threshold = threshold
    def __call__(self, data):
        corners = cv2.cornerHarris(data['image'], self.block_size, self.ksize, self.k)
        _, corners = cv2.threshold(corners, self.threshold*corners.max(), 255, 0)
        _, _, _, centroids = cv2.connectedComponentsWithStats(corners.astype(np.uint8))
        criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 100, 0.001)
        corners = cv2.cornerSubPix(data['image'], np.float32(centroids), (5,5), (-1,-1), criteria)
        res = np.hstack((centroids,corners))
        data['points'] = np.intp(res[1:, 2:4])
        return
    @staticmethod
    def draw(img, corners):
        for i in range(corners.shape[0]):
            cv2.circle(img, tuple(corners[i, :]), 5, (0, 0, 255), 2)
            cv2.circle(img, tuple(corners[i, :]), 2, (0, 255, 0), 2)
        return img


class SegmentsDetection:
    """ Segments detection
    """
    def __init__(self, threshold, min_length, max_gap, distance_step=1, angle_step=1) -> None:
        self.threshold = threshold
        self.min_length = min_length
        self.max_gap = max_gap
        self.angle_step = angle_step
        self.distance_step = distance_step

    def __call__(self, data):
        points_array = cv2.HoughLinesP(data['image'], self.distance_step,
            self.angle_step*np.pi/180, self.threshold,
            minLineLength=self.min_length, maxLineGap=self.max_gap).squeeze()
        segments = [LineSegment(*points) for points in points_array]
        if data.get('debug'):
            data['debug_image'] = data['original'].copy()
        if (canvas := data.get('debug_image', None)) is not None:
            colors = distinctipy.get_colors(len(segments))
            for color, segment in zip(colors, segments):
                color = list(map(lambda x: int(x*255), color))
                draw_line(canvas, segment, color, 3, markersize=10)
        data['segments'] = segments
        return data
