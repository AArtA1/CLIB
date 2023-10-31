#include "clib/Flexfixed.hpp"
#include "clib/logs.hpp"

namespace clib {

FlexFixed::FlexFixed(uint8_t I_n,uint8_t F_n): I(I_n), F(F_n){
    LOG(trace) << "Object successfully created";
}

FlexFixed::FlexFixed(uint8_t I_n, uint8_t F_n, uint8_t s_n, uint64_t n_n) : 
    I(I_n), F(F_n), s(s_n), n(n_n)
{
    if (!is_valid())
    {
        LOG(error) << "Can not create object. Invalid parameters";
        throw std::string{"Can not create object. Invalid parameters"};
    }

    LOG(trace) << "Object successfully created";
}


FlexFixed& FlexFixed::operator=(const FlexFixed& other){
    if(this != &other){
        I = other.I;
        F = other.F;
        s = other.s;
        n = other.n;
    }
    return *this;
}


// consider we already have res parameters: I and F 
void FlexFixed::multiplication(const FlexFixed& left,const FlexFixed& right, FlexFixed& res){
    LOG(trace) << "Flex multiplication of two numbers";


    if (!left.is_valid())
    {
        LOG(error) << "Left operand is invalid";
        throw std::string{"Invalid operand"};
    }

    if (!right.is_valid())
    {
        LOG(error) << "Right operand is invalid";
        throw std::string{"Invalid operand"};
    }

    LOG(trace) << "Left operand: " << left;
    LOG(trace) << "Right operand: " << right;

    res.s = left.s ^ right.s;

    int8_t delta_f = left.F - res.F + right.F;

    uint128_t res_n = left.n + right.n;

    if(delta_f >= 0){
        res_n = res_n >> delta_f;
    }
    else{
        res_n = res_n << abs(delta_f);
    }

    // overflow
    if(res_n >= static_cast<uint64_t>(1) << (res.I + res.F)){
        res_n = (static_cast<uint64_t>(1) << (res.I + res.F)) - 1;
    }


    res.n = res_n;

    LOG(trace) << "Result of flex multiplication: " << res;

}


void FlexFixed::addition(const FlexFixed& left,const FlexFixed& right, FlexFixed& res){
    
    LOG(trace) << "Flex addition of two numbers";


    if (!left.is_valid())
    {
        LOG(error) << "Left operand is invalid";
        throw std::string{"Invalid operand"};
    }

    if (!right.is_valid())
    {
        LOG(error) << "Right operand is invalid";
        throw std::string{"Invalid operand"};
    }



    LOG(trace) << "Left operand: " << left;
    LOG(trace) << "Right operand: " << right;

    uint8_t max_F = std::max(left.F, right.F);
    
    uint64_t left_n = left.n << (max_F - left.F), right_n = right.n << (max_F - right.F); 


    bool flag = false; 

    // check for |a| >= |b|
    // |a| < |b|
    if(!((left_n << max_F >= right_n << max_F) || (left_n << max_F == right_n << max_F && (1 << max_F - 1) & left_n >= (1 << max_F - 1) & right_n))){
        std::swap(left_n,right_n);
        flag = true;
    }

    uint128_t res_n = left.s == right.s?left_n + right_n:left_n - right_n;


    int8_t delta_F = max_F - res.F;

    if(delta_F >= 0 ){
        res_n = res_n >> delta_F;
    }
    else{
        res_n = res_n << delta_F;
    }

    res.s = flag?right.s:left.s;
    // overflow
    if(res_n >= static_cast<uint64_t>(1) << (res.I  + res.F)){
        res_n = static_cast<uint64_t>(1) << (res.I  + res.F) - 1;
    }

    res.n = res_n;

    LOG(trace) << "Result of flex addition: " << res;
}

void FlexFixed::substraction(const FlexFixed& left, const FlexFixed& right, FlexFixed& res){
    FlexFixed right_temp = right;

    if(right_temp.s == 0){
        right_temp.s = 1;
    }
    else{
        if(right_temp.s == 1){
            right_temp.s = 0;
        } 
    }

    FlexFixed::addition(left,right_temp,res);
}



bool operator>(const FlexFixed& left,const FlexFixed& right) {
    //todo for different signs
    return left.get_int() != right.get_int() ? left.get_int() > right.get_int() : left.get_frac() > right.get_frac();    
}




std::ostream& operator<<(std::ostream &oss, const FlexFixed &num)
{
    oss << "INT_WIDTH:" << static_cast<int>(num.I) << "\n";
    oss << "FRAC_WIDTH:" << static_cast<int>(num.F) << "\n";

    std::string sign_s = std::bitset<8>(num.s).to_string();
    std::string int_s = std::bitset<64>(num.get_int()).to_string();
    std::string frac_s = std::bitset<64>(num.get_frac()).to_string();

    sign_s = sign_s.substr(8 - 1, std::string::npos);
    int_s = int_s.substr(64 - num.I, std::string::npos);
    frac_s = frac_s.substr(64 - num.F, std::string::npos);

    oss << "Sign: " << sign_s << std::endl;
    oss << "Int:  " << int_s << std::endl;
    oss << "Frac: " << frac_s << std::endl;
    oss << "Presentation of flex_fixed " << int_s << "." << frac_s << std::endl;
    return oss;
}

bool FlexFixed::is_valid() const
{
    if (!(s <= 1))
        goto ivalid_obj;
    if (n > (static_cast<uint64_t>(1) << (I+F)) - 1)
        goto ivalid_obj;

    LOG(trace) << "Object is valid";
    return true;

ivalid_obj:
    LOG(error) << "Object is invalid";
    return false;
}

uint64_t FlexFixed::get_int() const{
    return n >> F;
}

uint64_t FlexFixed::get_frac() const{
    return ((static_cast<uint64_t>(1) << F) - 1) & n;
}

uint64_t FlexFixed::get_n() const{
    return n;
}

}