from functools import cached_property
from typing import NamedTuple
import numpy as np

class CourtDefinition(NamedTuple):
    length: float
    width: float
    serveline_width: float
    serveline_offset: float
    linewidth: float

"""
Tennis court

    |      |              |              |      |
    |      |              |              |      |
    +------+--------------+--------------+------+     <--  Net
    |      |              |              |      |
    |      |              |              |      |
    |      |              |              |      |
    |      |              |              |      |
    |      |              |              |      |
    |      |              |              |      |
    |      |              |              |      |
    |      |              |              |      |
    |      |              |              |      |
    |      A--------------E--------------B      |    <--  Service line
    |      |                             |      |
    |      |                             |      |
    |      |                             |      |
    |      |                             |      |
 y-axis    |                             |      |
    |      |                             |      |
 △  |      |                             |      |
 |  |      |                             |      |
    o------C-----------------------------D------+    <--  Baseline
(0,0)   -▷  x-axis
"""


court_definitions = {     # | length | width | serveline_width | serveline_offset | linewidth
    "ITF":  CourtDefinition(  23.77  , 10.97 ,      8.23       ,      6.40        ,    0.05    ),
}

class Court:
    def __init__(self, court_type: str):
        self.court_type = court_type
        self.court_definition = court_definitions[court_type]
    @cached_property
    def keypoints(self):
        Ax, Ay, _, Bx, By, _ = self.serveline.flatten()
        return np.array([
            [    Ax   , Ay , 0], # A
            [    Bx   , By , 0], # B
            [    Ax   ,  0 , 0], # C
            [    Bx   ,  0 , 0], # D
            [(Ax+Bx)/2, Ay , 0], # E
        ])
    @cached_property
    def centerline(self):
        x = self.court_definition.width/2
        length = self.court_definition.length
        serveline_offset = self.court_definition.serveline_offset
        return np.array([
            [x, length/2-serveline_offset, 0],
            [x, length/2+serveline_offset, 0]
        ])
    @cached_property
    def baseline(self):
        return np.array([
            [ 0 , 0, 0 ],
            [ self.court_definition.width, 0, 0]
        ])
    @cached_property
    def serveline(self):
        width = self.court_definition.width
        serveline_width = self.court_definition.serveline_width
        y = self.court_definition.length/2-self.court_definition.serveline_offset
        return np.array([
            [ (width-serveline_width)/2 , y, 0 ],
            [ (width+serveline_width)/2 , y, 0 ]
        ])
    @cached_property
    def leftsideline(self):
        return np.array([
            [ 0 , 0 , 0 ],
            [ 0 , self.court_definition.length, 0]
        ])
    @cached_property
    def rightsideline(self):
        return np.array([
            [ self.court_definition.width , 0 , 0],
            [ self.court_definition.width , self.court_definition.length, 0]
        ])
    @cached_property
    def leftsinglesideline(self):
        x = (self.court_definition.width - self.court_definition.serveline_width)/2
        return np.array([
            [ x , 0 , 0],
            [ x , self.court_definition.length, 0]
        ])
    @cached_property
    def rightsinglesideline(self):
        x = (self.court_definition.width + self.court_definition.serveline_width)/2
        return np.array([
            [ x , 0 , 0],
            [ x , self.court_definition.length, 0]
        ])
    @cached_property
    def netline(self):
        y = self.court_definition.length/2
        return np.array([
            [ 0 , y , 0],
            [ self.court_definition.width , y , 0]
        ])
    @cached_property
    def lines(self):
        return {
            "baseline": self.baseline,
            "serveline": self.serveline,
            "leftsideline": self.leftsideline,
            "rightsideline": self.rightsideline,
            "leftsinglesideline": self.leftsinglesideline,
            "rightsinglesideline": self.rightsinglesideline,
            "centerline": self.centerline,
            "netline": self.netline,
        }


