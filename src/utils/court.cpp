
#include <map>
#include "court.hpp"

std::map<std::string, CourtDefinition> court_definitions = {
    {"ITF", {23.77, 10.97, 8.23, 6.40, 0.05}},
};

Court::Court(std::string rule_type)
{
    this->court_definition = court_definitions[rule_type];
}

std::vector<cv::Point3f> Court::keypoints()
{
    std::vector<cv::Point3f> serveline = this->serveline();
    cv::Point3f A = serveline[0];
    cv::Point3f B = serveline[1];

    std::vector<cv::Point3f> keypoints(5);
    keypoints[0] = cv::Point3f(    A.x    , A.y , 0); // A
    keypoints[1] = cv::Point3f(    B.x    , B.y , 0); // B
    keypoints[2] = cv::Point3f(    A.x    ,  0  , 0); // C
    keypoints[3] = cv::Point3f(    B.x    ,  0  , 0); // D
    keypoints[4] = cv::Point3f((A.x+B.x)/2, A.y , 0); // E
    return keypoints;
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
