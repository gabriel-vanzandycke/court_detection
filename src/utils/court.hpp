#pragma once

#include <opencv2/core/mat.hpp>

/**
 * @brief Representation tennis court variable dimensions
 * @param length: length of the tennis court
 * @param width: width of the tennis court
 * @param serveline_width: width of the service line
 * @param serveline_offset: offset of the service line from the net
 * @param linewidth: tennis court lines width
 */
typedef struct {
    float length;
    float width;
    float serveline_width;
    float serveline_offset;
    float linewidth;
} CourtDefinition;


/*
Tennis court

        |    ◺- left sideline
        |   |          ◺- centerline
        |   |         |          ◺- right sideline
        |   |         |         |   |
        +---+---------+---------+---+    ◁--  Net
        |   |         |         |   |
        |   |         |         |   |
        |   |         |         |   |
        |   |         |         |   |
        |   |         |         |   |
        |   A---------E---------B   |    ◁--  Service line
        |   |                   |   |
        |   |                   |   |
        |   |                   |   |
y-axis  |   |                   |   |
     △  |   |                   |   |
     |  o---C-------------------D---+    ◁--  Baseline
    (0,0)--▷ x-axis
*/

/**
 * @brief Representation of a tennis court
 * @param rule_type: string representing the tennis court rule type (and its
 * dimensions). Currently, only ITF is supported.
 */
class Court
{
    public:
        Court(std::string rule_type);
        std::vector<cv::Point3f> netline();
        std::vector<cv::Point3f> baseline();
        std::vector<cv::Point3f> serveline();
        std::vector<cv::Point3f> centerline();
        std::vector<cv::Point3f> left_sideline();
        std::vector<cv::Point3f> right_sideline();
        std::vector<cv::Point3f> left_single_sideline();
        std::vector<cv::Point3f> right_single_sideline();
    private:
        CourtDefinition court_definition;
};

