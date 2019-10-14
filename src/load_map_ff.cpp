#include <ros/ros.h>
#include <fstream>
#include <iterator>

#include "ros/occupancy_grid_publisher.h"
#include "ros/init_utils.h"

#include "core/maps/grid_map.h"
#include "core/maps/plain_grid_map.h"
#include "slams/viny/viny_grid_cell.h"


int main(int  argc, char **argv) {
    ros::init(argc, argv, "load_map_ff");

    ros::NodeHandle nh;
    ros::Publisher pub = nh.advertise<nav_msgs::OccupancyGrid>("map", 5);
    std::string tf_frame = "odom_combined";
    auto gmp = MapValues::gmp;

    UnboundedPlainGridMap map = UnboundedPlainGridMap(std::make_shared<VinyDSCell>(), gmp);
    std::ifstream in("/home/dmo/Documents/dimplom/dumps/compressed_dump_8.txt");
    std::vector<char> file_content((std::istreambuf_iterator<char>(in)),
                                   std::istreambuf_iterator<char>());

    map.load_state(file_content);

    nav_msgs::OccupancyGrid map_msg;
    map_msg.header.frame_id = tf_frame;
    map_msg.info.map_load_time = ros::Time::now();
    map_msg.info.width = map.width();
    map_msg.info.height = map.height();
    map_msg.info.resolution = map.scale();
    // move map to the middle
    nav_msgs::MapMetaData &info = map_msg.info;
    DiscretePoint2D origin = map.origin();
    info.origin.position.x = -info.resolution * origin.x;
    info.origin.position.y = -info.resolution * origin.y;
    info.origin.position.z = 0;
    map_msg.data.reserve(info.height * info.width);
    DiscretePoint2D pnt;
    DiscretePoint2D end_of_map = DiscretePoint2D(info.width,
                                                 info.height) - origin;

    for (pnt.y = -origin.y; pnt.y < end_of_map.y; ++pnt.y) {
        for (pnt.x = -origin.x; pnt.x < end_of_map.x; ++pnt.x) {
            double value = static_cast<double>(map[pnt]);
            int cell_value = value == -1 ? -1 : static_cast<int>(value * 100);
            map_msg.data.push_back(cell_value);
        }
    }
    pub.publish(map_msg);
    ros::spinOnce();


    return 0;
}