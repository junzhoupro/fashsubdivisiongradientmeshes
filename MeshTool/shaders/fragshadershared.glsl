#version 410
//#extension GL_ARB_shading_language_420pack : enable
// Fragment shader

layout (location = 0) in vec3 colorOut;

uniform int colorBands;
uniform int computeDiff;
uniform float diffScaling;
uniform sampler2D texture0;

out vec4 fColor;

vec3 computePlasmaColormap(float value, vec3 p[256]) {
  return p[int(255 * clamp(value, 0, 1))];
}

float computeColorDistance(int i) {
  vec2 texSize;
  texSize = textureSize(texture0, 0);
  vec3 texColor;
  if(i == 1)
      texColor = texture(texture0, gl_FragCoord.xy / texSize / 2).xyz;
  else
      texColor = texture(texture0, gl_FragCoord.xy / texSize).xyz;
  return texColor == vec3(0) ? 0 : length(colorOut - texColor);
}

vec3 computeTextureColor() {
    vec2 texSize;
    texSize = textureSize(texture0, 0);
    vec3 texColor;
     texColor = texture(texture0, gl_FragCoord.xy / texSize/2).xyz;
//     texColor = texture(texture0, vec2(1,1)).xyz;

     return texColor;
}

void main() {
    vec3 plasmaColormap[256];

    plasmaColormap[0]=vec3(0.050383, 0.029803, 0.527975);
    plasmaColormap[1]=vec3(0.063536, 0.028426, 0.533124);
    plasmaColormap[2]=vec3(0.075353, 0.027206, 0.538007);
    plasmaColormap[3]=vec3(0.086222, 0.026125, 0.542658);
    plasmaColormap[4]=vec3(0.096379, 0.025165, 0.547103);
    plasmaColormap[5]=vec3(0.10598, 0.024309, 0.551368);
    plasmaColormap[6]=vec3(0.115124, 0.023556, 0.555468);
    plasmaColormap[7]=vec3(0.123903, 0.022878, 0.559423);
    plasmaColormap[8]=vec3(0.132381, 0.022258, 0.56325);
    plasmaColormap[9]=vec3(0.140603, 0.021687, 0.566959);
    plasmaColormap[10]=vec3(0.148607, 0.021154, 0.570562);
    plasmaColormap[11]=vec3(0.156421, 0.020651, 0.574065);
    plasmaColormap[12]=vec3(0.16407, 0.020171, 0.577478);
    plasmaColormap[13]=vec3(0.171574, 0.019706, 0.580806);
    plasmaColormap[14]=vec3(0.17895, 0.019252, 0.584054);
    plasmaColormap[15]=vec3(0.186213, 0.018803, 0.587228);
    plasmaColormap[16]=vec3(0.193374, 0.018354, 0.59033);
    plasmaColormap[17]=vec3(0.200445, 0.017902, 0.593364);
    plasmaColormap[18]=vec3(0.207435, 0.017442, 0.596333);
    plasmaColormap[19]=vec3(0.21435, 0.016973, 0.599239);
    plasmaColormap[20]=vec3(0.221197, 0.016497, 0.602083);
    plasmaColormap[21]=vec3(0.227983, 0.016007, 0.604867);
    plasmaColormap[22]=vec3(0.234715, 0.015502, 0.607592);
    plasmaColormap[23]=vec3(0.241396, 0.014979, 0.610259);
    plasmaColormap[24]=vec3(0.248032, 0.014439, 0.612868);
    plasmaColormap[25]=vec3(0.254627, 0.013882, 0.615419);
    plasmaColormap[26]=vec3(0.261183, 0.013308, 0.617911);
    plasmaColormap[27]=vec3(0.267703, 0.012716, 0.620346);
    plasmaColormap[28]=vec3(0.274191, 0.012109, 0.622722);
    plasmaColormap[29]=vec3(0.280648, 0.011488, 0.625038);
    plasmaColormap[30]=vec3(0.287076, 0.010855, 0.627295);
    plasmaColormap[31]=vec3(0.293478, 0.010213, 0.62949);
    plasmaColormap[32]=vec3(0.299855, 0.009561, 0.631624);
    plasmaColormap[33]=vec3(0.30621, 0.008902, 0.633694);
    plasmaColormap[34]=vec3(0.312543, 0.008239, 0.6357);
    plasmaColormap[35]=vec3(0.318856, 0.007576, 0.63764);
    plasmaColormap[36]=vec3(0.32515, 0.006915, 0.639512);
    plasmaColormap[37]=vec3(0.331426, 0.006261, 0.641316);
    plasmaColormap[38]=vec3(0.337683, 0.005618, 0.643049);
    plasmaColormap[39]=vec3(0.343925, 0.004991, 0.64471);
    plasmaColormap[40]=vec3(0.35015, 0.004382, 0.646298);
    plasmaColormap[41]=vec3(0.356359, 0.003798, 0.64781);
    plasmaColormap[42]=vec3(0.362553, 0.003243, 0.649245);
    plasmaColormap[43]=vec3(0.368733, 0.002724, 0.650601);
    plasmaColormap[44]=vec3(0.374897, 0.002245, 0.651876);
    plasmaColormap[45]=vec3(0.381047, 0.001814, 0.653068);
    plasmaColormap[46]=vec3(0.387183, 0.001434, 0.654177);
    plasmaColormap[47]=vec3(0.393304, 0.001114, 0.655199);
    plasmaColormap[48]=vec3(0.399411, 0.000859, 0.656133);
    plasmaColormap[49]=vec3(0.405503, 0.000678, 0.656977);
    plasmaColormap[50]=vec3(0.41158, 0.000577, 0.65773);
    plasmaColormap[51]=vec3(0.417642, 0.000564, 0.65839);
    plasmaColormap[52]=vec3(0.423689, 0.000646, 0.658956);
    plasmaColormap[53]=vec3(0.429719, 0.000831, 0.659425);
    plasmaColormap[54]=vec3(0.435734, 0.001127, 0.659797);
    plasmaColormap[55]=vec3(0.441732, 0.00154, 0.660069);
    plasmaColormap[56]=vec3(0.447714, 0.00208, 0.66024);
    plasmaColormap[57]=vec3(0.453677, 0.002755, 0.66031);
    plasmaColormap[58]=vec3(0.459623, 0.003574, 0.660277);
    plasmaColormap[59]=vec3(0.46555, 0.004545, 0.660139);
    plasmaColormap[60]=vec3(0.471457, 0.005678, 0.659897);
    plasmaColormap[61]=vec3(0.477344, 0.00698, 0.659549);
    plasmaColormap[62]=vec3(0.48321, 0.00846, 0.659095);
    plasmaColormap[63]=vec3(0.489055, 0.010127, 0.658534);
    plasmaColormap[64]=vec3(0.494877, 0.01199, 0.657865);
    plasmaColormap[65]=vec3(0.500678, 0.014055, 0.657088);
    plasmaColormap[66]=vec3(0.506454, 0.016333, 0.656202);
    plasmaColormap[67]=vec3(0.512206, 0.018833, 0.655209);
    plasmaColormap[68]=vec3(0.517933, 0.021563, 0.654109);
    plasmaColormap[69]=vec3(0.523633, 0.024532, 0.652901);
    plasmaColormap[70]=vec3(0.529306, 0.027747, 0.651586);
    plasmaColormap[71]=vec3(0.534952, 0.031217, 0.650165);
    plasmaColormap[72]=vec3(0.54057, 0.03495, 0.64864);
    plasmaColormap[73]=vec3(0.546157, 0.038954, 0.64701);
    plasmaColormap[74]=vec3(0.551715, 0.043136, 0.645277);
    plasmaColormap[75]=vec3(0.557243, 0.047331, 0.643443);
    plasmaColormap[76]=vec3(0.562738, 0.051545, 0.641509);
    plasmaColormap[77]=vec3(0.568201, 0.055778, 0.639477);
    plasmaColormap[78]=vec3(0.573632, 0.060028, 0.637349);
    plasmaColormap[79]=vec3(0.579029, 0.064296, 0.635126);
    plasmaColormap[80]=vec3(0.584391, 0.068579, 0.632812);
    plasmaColormap[81]=vec3(0.589719, 0.072878, 0.630408);
    plasmaColormap[82]=vec3(0.595011, 0.07719, 0.627917);
    plasmaColormap[83]=vec3(0.600266, 0.081516, 0.625342);
    plasmaColormap[84]=vec3(0.605485, 0.085854, 0.622686);
    plasmaColormap[85]=vec3(0.610667, 0.090204, 0.619951);
    plasmaColormap[86]=vec3(0.615812, 0.094564, 0.61714);
    plasmaColormap[87]=vec3(0.620919, 0.098934, 0.614257);
    plasmaColormap[88]=vec3(0.625987, 0.103312, 0.611305);
    plasmaColormap[89]=vec3(0.631017, 0.107699, 0.608287);
    plasmaColormap[90]=vec3(0.636008, 0.112092, 0.605205);
    plasmaColormap[91]=vec3(0.640959, 0.116492, 0.602065);
    plasmaColormap[92]=vec3(0.645872, 0.120898, 0.598867);
    plasmaColormap[93]=vec3(0.650746, 0.125309, 0.595617);
    plasmaColormap[94]=vec3(0.65558, 0.129725, 0.592317);
    plasmaColormap[95]=vec3(0.660374, 0.134144, 0.588971);
    plasmaColormap[96]=vec3(0.665129, 0.138566, 0.585582);
    plasmaColormap[97]=vec3(0.669845, 0.142992, 0.582154);
    plasmaColormap[98]=vec3(0.674522, 0.147419, 0.578688);
    plasmaColormap[99]=vec3(0.67916, 0.151848, 0.575189);
    plasmaColormap[100]=vec3(0.683758, 0.156278, 0.57166);
    plasmaColormap[101]=vec3(0.688318, 0.160709, 0.568103);
    plasmaColormap[102]=vec3(0.69284, 0.165141, 0.564522);
    plasmaColormap[103]=vec3(0.697324, 0.169573, 0.560919);
    plasmaColormap[104]=vec3(0.701769, 0.174005, 0.557296);
    plasmaColormap[105]=vec3(0.706178, 0.178437, 0.553657);
    plasmaColormap[106]=vec3(0.710549, 0.182868, 0.550004);
    plasmaColormap[107]=vec3(0.714883, 0.187299, 0.546338);
    plasmaColormap[108]=vec3(0.719181, 0.191729, 0.542663);
    plasmaColormap[109]=vec3(0.723444, 0.196158, 0.538981);
    plasmaColormap[110]=vec3(0.72767, 0.200586, 0.535293);
    plasmaColormap[111]=vec3(0.731862, 0.205013, 0.531601);
    plasmaColormap[112]=vec3(0.736019, 0.209439, 0.527908);
    plasmaColormap[113]=vec3(0.740143, 0.213864, 0.524216);
    plasmaColormap[114]=vec3(0.744232, 0.218288, 0.520524);
    plasmaColormap[115]=vec3(0.748289, 0.222711, 0.516834);
    plasmaColormap[116]=vec3(0.752312, 0.227133, 0.513149);
    plasmaColormap[117]=vec3(0.756304, 0.231555, 0.509468);
    plasmaColormap[118]=vec3(0.760264, 0.235976, 0.505794);
    plasmaColormap[119]=vec3(0.764193, 0.240396, 0.502126);
    plasmaColormap[120]=vec3(0.76809, 0.244817, 0.498465);
    plasmaColormap[121]=vec3(0.771958, 0.249237, 0.494813);
    plasmaColormap[122]=vec3(0.775796, 0.253658, 0.491171);
    plasmaColormap[123]=vec3(0.779604, 0.258078, 0.487539);
    plasmaColormap[124]=vec3(0.783383, 0.2625, 0.483918);
    plasmaColormap[125]=vec3(0.787133, 0.266922, 0.480307);
    plasmaColormap[126]=vec3(0.790855, 0.271345, 0.476706);
    plasmaColormap[127]=vec3(0.794549, 0.27577, 0.473117);
    plasmaColormap[128]=vec3(0.798216, 0.280197, 0.469538);
    plasmaColormap[129]=vec3(0.801855, 0.284626, 0.465971);
    plasmaColormap[130]=vec3(0.805467, 0.289057, 0.462415);
    plasmaColormap[131]=vec3(0.809052, 0.293491, 0.45887);
    plasmaColormap[132]=vec3(0.812612, 0.297928, 0.455338);
    plasmaColormap[133]=vec3(0.816144, 0.302368, 0.451816);
    plasmaColormap[134]=vec3(0.819651, 0.306812, 0.448306);
    plasmaColormap[135]=vec3(0.823132, 0.311261, 0.444806);
    plasmaColormap[136]=vec3(0.826588, 0.315714, 0.441316);
    plasmaColormap[137]=vec3(0.830018, 0.320172, 0.437836);
    plasmaColormap[138]=vec3(0.833422, 0.324635, 0.434366);
    plasmaColormap[139]=vec3(0.836801, 0.329105, 0.430905);
    plasmaColormap[140]=vec3(0.840155, 0.33358, 0.427455);
    plasmaColormap[141]=vec3(0.843484, 0.338062, 0.424013);
    plasmaColormap[142]=vec3(0.846788, 0.342551, 0.420579);
    plasmaColormap[143]=vec3(0.850066, 0.347048, 0.417153);
    plasmaColormap[144]=vec3(0.853319, 0.351553, 0.413734);
    plasmaColormap[145]=vec3(0.856547, 0.356066, 0.410322);
    plasmaColormap[146]=vec3(0.85975, 0.360588, 0.406917);
    plasmaColormap[147]=vec3(0.862927, 0.365119, 0.403519);
    plasmaColormap[148]=vec3(0.866078, 0.36966, 0.400126);
    plasmaColormap[149]=vec3(0.869203, 0.374212, 0.396738);
    plasmaColormap[150]=vec3(0.872303, 0.378774, 0.393355);
    plasmaColormap[151]=vec3(0.875376, 0.383347, 0.389976);
    plasmaColormap[152]=vec3(0.878423, 0.387932, 0.3866);
    plasmaColormap[153]=vec3(0.881443, 0.392529, 0.383229);
    plasmaColormap[154]=vec3(0.884436, 0.397139, 0.37986);
    plasmaColormap[155]=vec3(0.887402, 0.401762, 0.376494);
    plasmaColormap[156]=vec3(0.89034, 0.406398, 0.37313);
    plasmaColormap[157]=vec3(0.89325, 0.411048, 0.369768);
    plasmaColormap[158]=vec3(0.896131, 0.415712, 0.366407);
    plasmaColormap[159]=vec3(0.898984, 0.420392, 0.363047);
    plasmaColormap[160]=vec3(0.901807, 0.425087, 0.359688);
    plasmaColormap[161]=vec3(0.904601, 0.429797, 0.356329);
    plasmaColormap[162]=vec3(0.907365, 0.434524, 0.35297);
    plasmaColormap[163]=vec3(0.910098, 0.439268, 0.34961);
    plasmaColormap[164]=vec3(0.9128, 0.444029, 0.346251);
    plasmaColormap[165]=vec3(0.915471, 0.448807, 0.34289);
    plasmaColormap[166]=vec3(0.918109, 0.453603, 0.339529);
    plasmaColormap[167]=vec3(0.920714, 0.458417, 0.336166);
    plasmaColormap[168]=vec3(0.923287, 0.463251, 0.332801);
    plasmaColormap[169]=vec3(0.925825, 0.468103, 0.329435);
    plasmaColormap[170]=vec3(0.928329, 0.472975, 0.326067);
    plasmaColormap[171]=vec3(0.930798, 0.477867, 0.322697);
    plasmaColormap[172]=vec3(0.933232, 0.48278, 0.319325);
    plasmaColormap[173]=vec3(0.93563, 0.487712, 0.315952);
    plasmaColormap[174]=vec3(0.93799, 0.492667, 0.312575);
    plasmaColormap[175]=vec3(0.940313, 0.497642, 0.309197);
    plasmaColormap[176]=vec3(0.942598, 0.502639, 0.305816);
    plasmaColormap[177]=vec3(0.944844, 0.507658, 0.302433);
    plasmaColormap[178]=vec3(0.947051, 0.512699, 0.299049);
    plasmaColormap[179]=vec3(0.949217, 0.517763, 0.295662);
    plasmaColormap[180]=vec3(0.951344, 0.52285, 0.292275);
    plasmaColormap[181]=vec3(0.953428, 0.52796, 0.288883);
    plasmaColormap[182]=vec3(0.95547, 0.533093, 0.28549);
    plasmaColormap[183]=vec3(0.957469, 0.53825, 0.282096);
    plasmaColormap[184]=vec3(0.959424, 0.543431, 0.278701);
    plasmaColormap[185]=vec3(0.961336, 0.548636, 0.275305);
    plasmaColormap[186]=vec3(0.963203, 0.553865, 0.271909);
    plasmaColormap[187]=vec3(0.965024, 0.559118, 0.268513);
    plasmaColormap[188]=vec3(0.966798, 0.564396, 0.265118);
    plasmaColormap[189]=vec3(0.968526, 0.5697, 0.261721);
    plasmaColormap[190]=vec3(0.970205, 0.575028, 0.258325);
    plasmaColormap[191]=vec3(0.971835, 0.580382, 0.254931);
    plasmaColormap[192]=vec3(0.973416, 0.585761, 0.25154);
    plasmaColormap[193]=vec3(0.974947, 0.591165, 0.248151);
    plasmaColormap[194]=vec3(0.976428, 0.596595, 0.244767);
    plasmaColormap[195]=vec3(0.977856, 0.602051, 0.241387);
    plasmaColormap[196]=vec3(0.979233, 0.607532, 0.238013);
    plasmaColormap[197]=vec3(0.980556, 0.613039, 0.234646);
    plasmaColormap[198]=vec3(0.981826, 0.618572, 0.231287);
    plasmaColormap[199]=vec3(0.983041, 0.624131, 0.227937);
    plasmaColormap[200]=vec3(0.984199, 0.629718, 0.224595);
    plasmaColormap[201]=vec3(0.985301, 0.63533, 0.221265);
    plasmaColormap[202]=vec3(0.986345, 0.640969, 0.217948);
    plasmaColormap[203]=vec3(0.987332, 0.646633, 0.214648);
    plasmaColormap[204]=vec3(0.98826, 0.652325, 0.211364);
    plasmaColormap[205]=vec3(0.989128, 0.658043, 0.2081);
    plasmaColormap[206]=vec3(0.989935, 0.663787, 0.204859);
    plasmaColormap[207]=vec3(0.990681, 0.669558, 0.201642);
    plasmaColormap[208]=vec3(0.991365, 0.675355, 0.198453);
    plasmaColormap[209]=vec3(0.991985, 0.681179, 0.195295);
    plasmaColormap[210]=vec3(0.992541, 0.68703, 0.19217);
    plasmaColormap[211]=vec3(0.993032, 0.692907, 0.189084);
    plasmaColormap[212]=vec3(0.993456, 0.69881, 0.186041);
    plasmaColormap[213]=vec3(0.993814, 0.704741, 0.183043);
    plasmaColormap[214]=vec3(0.994103, 0.710698, 0.180097);
    plasmaColormap[215]=vec3(0.994324, 0.716681, 0.177208);
    plasmaColormap[216]=vec3(0.994474, 0.722691, 0.174381);
    plasmaColormap[217]=vec3(0.994553, 0.728728, 0.171622);
    plasmaColormap[218]=vec3(0.994561, 0.734791, 0.168938);
    plasmaColormap[219]=vec3(0.994495, 0.74088, 0.166335);
    plasmaColormap[220]=vec3(0.994355, 0.746995, 0.163821);
    plasmaColormap[221]=vec3(0.994141, 0.753137, 0.161404);
    plasmaColormap[222]=vec3(0.993851, 0.759304, 0.159092);
    plasmaColormap[223]=vec3(0.993482, 0.765499, 0.156891);
    plasmaColormap[224]=vec3(0.993033, 0.77172, 0.154808);
    plasmaColormap[225]=vec3(0.992505, 0.777967, 0.152855);
    plasmaColormap[226]=vec3(0.991897, 0.784239, 0.151042);
    plasmaColormap[227]=vec3(0.991209, 0.790537, 0.149377);
    plasmaColormap[228]=vec3(0.990439, 0.796859, 0.14787);
    plasmaColormap[229]=vec3(0.989587, 0.803205, 0.146529);
    plasmaColormap[230]=vec3(0.988648, 0.809579, 0.145357);
    plasmaColormap[231]=vec3(0.987621, 0.815978, 0.144363);
    plasmaColormap[232]=vec3(0.986509, 0.822401, 0.143557);
    plasmaColormap[233]=vec3(0.985314, 0.828846, 0.142945);
    plasmaColormap[234]=vec3(0.984031, 0.835315, 0.142528);
    plasmaColormap[235]=vec3(0.982653, 0.841812, 0.142303);
    plasmaColormap[236]=vec3(0.98119, 0.848329, 0.142279);
    plasmaColormap[237]=vec3(0.979644, 0.854866, 0.142453);
    plasmaColormap[238]=vec3(0.977995, 0.861432, 0.142808);
    plasmaColormap[239]=vec3(0.976265, 0.868016, 0.143351);
    plasmaColormap[240]=vec3(0.974443, 0.874622, 0.144061);
    plasmaColormap[241]=vec3(0.97253, 0.88125, 0.144923);
    plasmaColormap[242]=vec3(0.970533, 0.887896, 0.145919);
    plasmaColormap[243]=vec3(0.968443, 0.894564, 0.147014);
    plasmaColormap[244]=vec3(0.966271, 0.901249, 0.14818);
    plasmaColormap[245]=vec3(0.964021, 0.90795, 0.14937);
    plasmaColormap[246]=vec3(0.961681, 0.914672, 0.15052);
    plasmaColormap[247]=vec3(0.959276, 0.921407, 0.151566);
    plasmaColormap[248]=vec3(0.956808, 0.928152, 0.152409);
    plasmaColormap[249]=vec3(0.954287, 0.934908, 0.152921);
    plasmaColormap[250]=vec3(0.951726, 0.941671, 0.152925);
    plasmaColormap[251]=vec3(0.949151, 0.948435, 0.152178);
    plasmaColormap[252]=vec3(0.946602, 0.95519, 0.150328);
    plasmaColormap[253]=vec3(0.944152, 0.961916, 0.146861);
    plasmaColormap[254]=vec3(0.941896, 0.96859, 0.140956);
    plasmaColormap[255]=vec3(0.940015, 0.975158, 0.131326);

  if (computeDiff == 0) // Compute color
    fColor = colorBands == 0 ? vec4(colorOut, 1) : vec4(floor(.5 + colorBands * colorOut) / colorBands, 1);
  else if (computeDiff == 1) // Compute colormapped distance
      fColor = vec4(computePlasmaColormap(diffScaling * computeColorDistance(1), plasmaColormap), 1); //computeColorDistance(2) if you don't use MAC.
  else if (computeDiff == 2) { // Compute distance
    float distance = computeColorDistance(2);
    float r = floor(distance * 255);
    float g = floor((distance - r / 255) * pow(255, 2));
    float b = floor((distance - r / 255 - g / pow(255, 2)) * pow(255, 3));
    fColor = vec4(r / 255, g / 255, b / 255, 1);
  }

//  fColor = vec4(0, 0, 0, 1);
}
