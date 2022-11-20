//Must match the defines in the atmosphere_common.hlsl shader
#define TRANSMITTANCE_TEXTURE_WIDTH 256
#define TRANSMITTANCE_TEXTURE_HEIGHT 64

#define IRRADIANCE_TEXTURE_WIDTH 64
#define IRRADIANCE_TEXTURE_HEIGHT 16

#define SCATTERING_TEXTURE_R_SIZE 32
#define SCATTERING_TEXTURE_MU_SIZE 128
#define SCATTERING_TEXTURE_MU_S_SIZE 32
#define SCATTERING_TEXTURE_NU_SIZE 8

#define SCATTERING_TEXTURE_WIDTH  (SCATTERING_TEXTURE_NU_SIZE * SCATTERING_TEXTURE_MU_S_SIZE)
#define SCATTERING_TEXTURE_HEIGHT (SCATTERING_TEXTURE_MU_SIZE)
#define SCATTERING_TEXTURE_DEPTH  (SCATTERING_TEXTURE_R_SIZE)


#define MAX_SCATTERING_ORDER 4

// An atmosphere layer of width 'width', and whose density is defined as
// 'exp_term' * exp('exp_scale' * h) + 'linear_term' * h + 'constant_term',
// clamped to [0,1], and where h is the altitude.
struct density_layer 
{
    float ExpTerm;
    float ExpScale;
    float LinearTerm;
    float ConstantTerm;
};

//Atmosphere parameters (must match the one used in atmosphere_*.hlsl shaders, including padding rules for hlsl)
//All radiuses and heights are in km
struct atmosphere_parameters
{
    float Rg;  //Radius of ground
    float Rt;  //Radius of top of atmosphere
    float Hr;  //Rayleigh height scale
    float Hm;  //Mie height scale
        
    vec3 Bsr;             //Rayleigh scattering coefficient
    float __padding0;
    vec3 Ber;             //Rayleigh extintion coefficient
    float __padding1;
    density_layer RLayer; //Rayleigh density layer
    
    vec3 Bsm;             //Mie scattering coefficient
    float __padding2;
    vec3 Bem;             //Mie extintion coefficient
    float MieG;
    density_layer MLayer; //Mie density layer
    
    vec3 Beo;                 //Ozone-absorption extintion coefficient
    float OLayerHeight;       //Height at which we change the ozone density layer
    density_layer OLayers[2]; //Ozone-absorption density layers
    
    vec3 SolarIrradiance;
    float SunAngularRadius;
    
    float MuSMin;
    vec3 GroundAlbedo;
};
