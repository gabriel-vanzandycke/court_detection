from functools import cached_property

import cv2
import numpy as np
import matplotlib.pyplot as plt

EPS = np.finfo(np.float32).eps

def load_image(path, width, height):
    f = open(path, "rb")
    shape = (height, width)
    return np.frombuffer(f.read(width*height), dtype=np.uint8).reshape(shape)

def display_image(image, title=None):
    fig = plt.figure(figsize=np.array([5,1.5])*2)
    ax = fig.gca()
    res = ax.imshow(image)
    if title:
        ax.set_title(title)
    fig.colorbar(res)
    plt.show()

def draw_line(image, line, color=(0, 0, 255), thickness=2, markersize=5, label=""):
    cv2.line(image, (int(line.x1), int(line.y1)), (int(line.x2), int(line.y2)), color, thickness)
    cv2.circle(image, (int(line.x1), int(line.y1)), markersize, color, -1)
    cv2.circle(image, (int(line.x2), int(line.y2)), markersize, color, -1)
    x, y = int((line.x1 + line.x2)/2), int((line.y1 + line.y2)/2)
    cv2.putText(image, label, (x, y), cv2.FONT_HERSHEY_SIMPLEX, 1, color, thickness)


class LineSegment:
    """          y2-y1
        y - y1 = ----- (x - x1)
                 x2-x1
    """
    def __init__(self, x1, y1, x2, y2):
        self.x1, self.y1, self.x2, self.y2 = x1, y1, x2, y2
        self.theta = np.pi - np.arctan2(self.x2-self.x1, self.y2-self.y1)
        self.rho = self.x1*np.cos(self.theta) + self.y1*np.sin(self.theta)
        if self.rho < 0:
            self.rho *= -1
            self.theta -= np.pi
    @property
    def slope(self):
        denominator = self.x2-self.x1
        return (self.y2-self.y1)/denominator if np.abs(denominator) > EPS else np.inf
    @property
    def x_intercept(self):
        denominator = self.y2-self.y1
        return self.x1 - self.y1 * (self.x2-self.x1)/denominator if np.abs(denominator) > EPS else np.inf
    @property
    def y_intercept(self):
        denominator = self.x2-self.x1
        return self.y1 - self.x1 * (self.y2-self.y1)/denominator if np.abs(denominator) > EPS else np.inf
    @property
    def orientation(self):
        return "vertical" if np.abs(self.slope) > 1.0 else "horizontal"
    @property
    def length(self):
        return np.sqrt((self.x2-self.x1)**2 + (self.y2-self.y1)**2)
    def __str__(self):
        return f"LineSegment(x1={self.x1}, y1={self.y1}, x2={self.x2}, y2={self.y2})"

def closest_point(rho, theta, x, y):
    """ Find the closest point on a line to a point
        Line is defined in Hough space by `rho` and `theta`
        Point is defined by `x` and `y` in carthesian coordinates
    """
    b = rho*np.array([[np.cos(theta)], [np.sin(theta)]])
    a = np.array([[x], [y]])
    alpha = theta - np.arctan2(y, x) # angle between `a` and `b`
    return b + a - np.linalg.norm(a)*np.cos(alpha)*b/np.linalg.norm(b)

def distance_to_line(rho, theta, x, y):
    """ Find the distance from a point to a line
        Line is defined in Hough space by `rho` and `theta`
        Point is defined by `x` and `y` in carthesian coordinates
    """
    return np.linalg.norm(closest_point(rho, theta, x, y) - np.array([[x], [y]])) # TODO: optimize

def lines_intersection(rho1, theta1, rho2, theta2):
    """ Find the intersection point of two lines
        Lines are defined in Hough space by `rho` and `theta`
    """
    A = np.array([[np.cos(theta1), np.sin(theta1)], [np.cos(theta2), np.sin(theta2)]])
    b = np.array([[rho1], [rho2]])
    return np.linalg.inv(A) @ b
