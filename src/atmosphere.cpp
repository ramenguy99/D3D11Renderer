#include "atmosphere.h"

internal atmosphere_parameters
GetEarthAtmosphereParameters()
{
    atmosphere_parameters Result = {};
    Result.Rg = 6360.f;
    Result.Rt = 6460.f;
    Result.Hr = 8.0f;
    Result.Hm = 1.2f;
    
    Result.Bsr = vec3(5.8e-3f, 1.35e-2f, 3.31e-2f);
    Result.Ber = Result.Bsr;
    Result.RLayer = {1.0f, -1.0f / Result.Hr, 0.0f, 0.0f};
    
    Result.Bsm = vec3(3.996e-3f);
    Result.Bem = vec3(4.440e-3f);
    Result.MieG = 0.8f;
    Result.MLayer = {1.0f, -1.0f / Result.Hm, 0.0f, 0.0f};
    
    Result.Beo = vec3(6.50e-4f, 1.881e-3f, 8.5e-5f);
    Result.OLayerHeight = 25.0f;
    Result.OLayers[0] = {0.0f, 0.0f, 1.0f / 15.0f, -2.0f / 3.0f};
    Result.OLayers[1] = {0.0f, 0.0f, -1.0f / 15.0f, 8.0f / 3.0f};
    
    //Result.SolarIrradiance = vec3(1.474000f, 1.850400f, 1.911980f);
    // Using a normalise sun illuminance. 
    // This is to make sure the LUTs acts as a transfert factor to apply the runtime computed sun irradiance over.
    Result.SolarIrradiance = vec3(1.0f, 1.0f, 1.0f);	
    Result.SunAngularRadius = 0.004675f;
    Result.MuSMin = (f32)cos(PI * 120.0 / 180.0);
    Result.GroundAlbedo = vec3(0.5f);
    
    return Result;
}