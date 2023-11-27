#include <future>

#include "clib/converter.hpp"

#include "clib/logs.hpp"

namespace clib
{

// template<typename T,typename Y>
// std::vector<std::vector<std::vector<Y>>> converter::convert_to_type(const CImg<T>& pixels,const Y& hyperparams){
//     const size_t width = pixels.width(), height = pixels.height();

//     std::vector<std::vector<std::vector<Y>>> res_pixels;    
//     std::vector<std::future<void>> future_results(width);

//     for(size_t i = 0; i < width;++i){
//         results[i] = std::async([&](const CImg<T>& pixels, int i){  })
//     }    

//     for(size_t i = 0; i < width;++i){
//         for(size_t j = 0; j < height;++j){

//         }



//         for(size_t j = 0; j < height;++j){
//             float r = *Image.data(i,j,0,0), g = *Image.data(i,j,0,1), b = *Image.data(i,j,0,2);
//             pixels[i][j] = Y::from_float(hyperparams,std::max(r,std::max(g,b)));
//         }
//     }
// }


// std::vector<std::vector<Flexfloat>> converter::convert_from_ff_to_gscale(std::vector<std::vector<std::vector<Flexfloat>>>& pixels){

// }

// std::vector<std::vector<Flexfixed>> converter::convert_from_fx_to_gscale(std::vector<std::vector<std::vector<Flexfixed>>>& pixels){

// }

}