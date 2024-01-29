
static map<string, vector<vector<coef_t>>> COEFS{
    {"inv",
     {{16, -83248, 124263, -130720, 65531},
      {16, -37043, 92455, -123039, 64880},
      {16, -18907, 66153, -110171, 62755},
      {16, -10636, 47880, -96643, 59398}}},

    {"exp2",
     {{16, 374, 3290, 15500, 45340, -11},
      {16, 530, 3594, 15735, 45425, 0},
      {16, 750, 3582, 15754, 45425, 0},
      {16, 1062, 2939, 16277, 45228, 28}}},

    {"log2", {{16, -37615, 93656, 17}, {16, -25105, 87706, 750}, {16, -17941, 80679, 2487}, {16, -13459, 74024, 4967}}},
    {"sqrt",
     {{16, -2079, 4045, -8189, 32767, 0},
      {16, -1407, 3724, -8128, 32762, 0},
      {16, -990, 3315, -7976, 32736, 1},
      {16, -719, 2914, -7751, 32680, 7},
      {16, -537, 2552, -7481, 32590, 18},
      {16, -410, 2236, -7185, 32467, 37},
      {16, -319, 1964, -6880, 32315, 66},
      {16, -252, 1732, -6576, 32137, 105}}},

    {"sqrt2",
     {{16, -2425, 5554, -11564, 46340, 27145},
      {16, -1195, 4417, -11145, 46267, 27150},
      {16, -664, 3386, -10385, 46014, 27182},
      {16, -401, 2611, -9523, 45585, 27263}}},

    {"cos",
     {{16, 8180, -82146, 70, 65535},
      {16, 23451, -93423, 2981, 65273},
      {16, 35135, -110673, 11573, 63830},
      {16, 41443, -124518, 21752, 61323}}},

    {"sin",
     {{16, -41443, -188, 102955, 0},
      {16, -35135, -5267, 104367, -134},
      {16, -23451, -23068, 113509, -1716},
      {16, -8180, -57606, 139682, -8361}}},

    {"ctan",
     {{16, 9230, -77, 53905, 0},
      {16, 12247, -2552, 54605, -67},
      {16, 20731, -15753, 61522, -1286},
      {16, 43580, -68603, 102457, -11900}}},

    {"asin",
     {{16, 7517, -120, 41728, 0},
      {16, 13239, -4925, 43113, -135},
      {16, 45278, -56500, 71001, -5194},
      {16, 1887697, -4681702, 3937130, -1080961}}}};