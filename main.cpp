#include <iostream>

#include <utils.hpp>
#include <image_processing.hpp>

int main(int argc, char *argv[]){
    Blur blur(5);
    std::cout << dummy() << blur.kernel << std::endl;
    return 0;
}
