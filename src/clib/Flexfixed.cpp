#include "clib/Flexfixed.hpp"
#include "clib/logs.hpp"

namespace clib {

Flexfixed::Flexfixed(Itype I_n,Ftype F_n): I(I_n), F(F_n){
    LOG(trace) << "Object successfully created";
}

Flexfixed::Flexfixed(Itype I_n, Ftype F_n, stype s_n, ntype n_n) : 
    I(I_n), F(F_n), s(s_n), n(n_n)
{
    if (!is_valid())
    {
        LOG(error) << "Can not create object. Invalid parameters";
        throw std::string{"Can not create object. Invalid parameters"};
    }

    LOG(trace) << "Object successfully created";
}


Flexfixed& Flexfixed::operator=(const Flexfixed& other){
    if(this != &other){
        I = other.I;
        F = other.F;
        s = other.s;
        n = other.n;
    }
    return *this;
}


// consider we already have res parameters: I and F 
void Flexfixed::multiplication(const Flexfixed& left,const Flexfixed& right, Flexfixed& res){
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

    int16_t delta_f = static_cast<int16_t>(left.F) + right.F - res.F;

    ntype res_n = left.n + right.n;

    if(delta_f >= 0){
        res_n = res_n >> delta_f;
    }
    else{
        res_n = res_n << abs(delta_f);
    }

    // overflow
    if(res_n >= static_cast<ntype>(1) << (res.I + res.F)){
        res_n = (static_cast<ntype>(1) << (res.I + res.F)) - 1;
    }


    res.n = res_n;

    LOG(trace) << "Result of flex multiplication: " << res;

}


void Flexfixed::addition(const Flexfixed& left,const Flexfixed& right, Flexfixed& res){
    
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

    Ftype max_F = std::max(left.F, right.F);
    
    ntype left_n = left.n << (max_F - left.F), right_n = right.n << (max_F - right.F); 


    bool flag = false; 

    // check for |a| >= |b|
    // |a| < |b|
    if(!((left_n << max_F >= right_n << max_F) || (left_n << max_F == right_n << max_F && (((static_cast<ntype>(1) << max_F) - 1) & left_n) >= (((static_cast<ntype>(1) << max_F) - 1) & right_n)))){
        std::swap(left_n,right_n);
        flag = true;
    }

    ntype res_n = left.s == right.s?left_n + right_n:left_n - right_n;


    int16_t delta_F = static_cast<int16_t>(max_F) - res.F;

    if(delta_F >= 0 ){
        res_n = res_n >> delta_F;
    }
    else{
        res_n = res_n << delta_F;
    }

    res.s = flag?right.s:left.s;
    // overflow
    if(res_n >= static_cast<ntype>(1) << (res.I  + res.F)){
        res_n = (static_cast<ntype>(1) << (res.I  + res.F)) - 1;
    }

    res.n = res_n;

    LOG(trace) << "Result of flex addition: " << res;
}

void Flexfixed::substraction(const Flexfixed& left, const Flexfixed& right, Flexfixed& res){
    Flexfixed right_temp = right;

    if(right_temp.s == 0){
        right_temp.s = 1;
    }
    else{
        if(right_temp.s == 1){
            right_temp.s = 0;
        } 
    }

    Flexfixed::addition(left,right_temp,res);
}



bool operator>(const Flexfixed& left,const Flexfixed& right) {
    //todo for different signs
    return left.get_int() != right.get_int() ? left.get_int() > right.get_int() : left.get_frac() > right.get_frac();    
}




std::ostream& operator<<(std::ostream &oss, const Flexfixed &num)
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

bool Flexfixed::is_valid() const
{
    if (!(s <= 1))
        goto ivalid_obj;
    if (n > (static_cast<ntype>(1) << (I+F)) - 1)
        goto ivalid_obj;

    LOG(trace) << "Object is valid";
    return true;

ivalid_obj:
    LOG(error) << "Object is invalid";
    return false;
}

ntype Flexfixed::get_int() const{
    return n >> F;
}

ntype Flexfixed::get_frac() const{
    return ((static_cast<ntype>(1) << F) - 1) & n;
}

inline ntype Flexfixed::get_n() const{
    return n;
}

}