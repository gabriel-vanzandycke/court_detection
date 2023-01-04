#pragma once

#include <string>

std::string dummy();


// void draw_line(Mat& image, Line line, Vec3b color=Vec3b(0, 0, 255), int thickness=2, int markersize=5, string label="") {
//     cv::line(image, Point(int(line.x1), int(line.y1)), Point(int(line.x2), int(line.y2)), color, thickness);
//     cv::circle(image, Point(int(line.x1), int(line.y1)), markersize, color, -1);
//     cv::circle(image, Point(int(line.x2), int(line.y2)), markersize, color, -1);
//     int x = int((line.x1 + line.x2)/2), int y = int((line.y1 + line.y2)/2);
//     cv::putText(image, label, Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 1, color, thickness);
// }
