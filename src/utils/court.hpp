#pragma once

#include <opencv2/core/mat.hpp>

typedef struct {
    float length;
    float width;
    float serveline_width;
    float serveline_offset;
    float linewidth;
} CourtDefinition;


class Court
{
    public:
        Court(std::string rule_type);
        std::vector<cv::Point3f> keypoints();
    private:
        CourtDefinition court_definition;
        std::vector<cv::Point3f> centerline();
        std::vector<cv::Point3f> serveline();
};
