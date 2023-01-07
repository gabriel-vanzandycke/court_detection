
#include <map>
#include "court.hpp"

std::map<std::string, CourtDefinition> court_definitions = {
    {"ITF", {23.77, 10.97, 8.23, 6.40, 0.05}},
};

Court::Court(std::string rule_type)
{
    this->court_definition = court_definitions[rule_type];
}

std::vector<cv::Point3f> Court::serveline()
{
    float width = this->court_definition.width;
    float serveline_width = this->court_definition.serveline_width;
    float y = this->court_definition.length/2-this->court_definition.serveline_offset;

    std::vector<cv::Point3f> keypoints(2);
    keypoints = {cv::Point3f((width-serveline_width)/2, y, 0),
                 cv::Point3f((width+serveline_width)/2, y, 0)};
    return keypoints;
}

std::vector<cv::Point3f> Court::baseline()
{
    float width = this->court_definition.width;

    std::vector<cv::Point3f> keypoints(2);
    keypoints = {cv::Point3f(0, 0, 0),
                 cv::Point3f(width, 0, 0)};
    return keypoints;
}

std::vector<cv::Point3f> Court::netline()
{
    float width = this->court_definition.width;
    float y = this->court_definition.length/2;

    std::vector<cv::Point3f> keypoints(2);
    keypoints = {cv::Point3f(0, y, 0),
                 cv::Point3f(width, y, 0)};
    return keypoints;
}

std::vector<cv::Point3f> Court::centerline()
{
    float x = this->court_definition.width/2;
    float mid = this->court_definition.length/2;
    float offset = this->court_definition.serveline_offset;

    std::vector<cv::Point3f> keypoints(2);
    keypoints = {cv::Point3f(x, mid-offset, 0),
                 cv::Point3f(x, mid+offset, 0)};
    return keypoints;
}

std::vector<cv::Point3f> Court::left_sideline()
{
    float length = this->court_definition.length;

    std::vector<cv::Point3f> keypoints(2);
    keypoints = {cv::Point3f(0, 0, 0),
                 cv::Point3f(0, length, 0)};
    return keypoints;
}

std::vector<cv::Point3f> Court::right_sideline()
{
    float width = this->court_definition.width;
    float length = this->court_definition.length;

    std::vector<cv::Point3f> keypoints(2);
    keypoints = {cv::Point3f(width, 0, 0),
                 cv::Point3f(width, length, 0)};
    return keypoints;
}

std::vector<cv::Point3f> Court::left_single_sideline()
{
    float x = (this->court_definition.width + this->court_definition.serveline_width)/2;
    float length = this->court_definition.length;

    std::vector<cv::Point3f> keypoints(2);
    keypoints = {cv::Point3f(x, 0, 0),
                 cv::Point3f(x, length, 0)};
    return keypoints;
}

std::vector<cv::Point3f> Court::right_single_sideline()
{
    float x = (this->court_definition.width - this->court_definition.serveline_width)/2;
    float length = this->court_definition.length;

    std::vector<cv::Point3f> keypoints(2);
    keypoints = {cv::Point3f(x, 0, 0),
                 cv::Point3f(x, length, 0)};
    return keypoints;
}
