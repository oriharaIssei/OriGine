
/// Reference
// https://www.ronja-tutorials.com/post/024-white-noise/

float rand3dTo1d(float3 _input, float3 _dotDir = float3(12.9898, 78.233, 37.719))
{
    float random = dot(sin(_input), _dotDir);
    random = frac(sin(random) * 143758.5453);
    return random;
}

float rand2dTo1d(float2 _input, float2 _dotDir = float2(12.9898, 78.233))
{
    float random = dot(sin(_input), _dotDir);
    random = frac(sin(random) * 143758.5453);
    return random;
}

float rand1dTo1d(float _input, float _mutator = 0.546)
{
    float random = frac(sin(_input + _mutator) * 143758.5453);
    return random;
}

/// ------------------------------------------------------------------
// Rand3d To

float3 rand3dTo3d(float3 _input)
{
    return float3(
        rand3dTo1d(_input, float3(12.989, 78.233, 37.719)),
        rand3dTo1d(_input, float3(39.346, 11.135, 83.155)),
        rand3dTo1d(_input, float3(73.156, 52.235, 09.151))
    );
}

float2 rand3dTo2d(float3 _input)
{
    return float2(
        rand3dTo1d(_input, float3(12.989, 78.233, 37.719)),
        rand3dTo1d(_input, float3(39.346, 11.135, 83.155))
    );
}

/// ------------------------------------------------------------------
// Rand2d To

float3 rand2dTo3d(float2 _input)
{
    return float3(
        rand2dTo1d(_input, float2(12.989, 78.233)),
        rand2dTo1d(_input, float2(39.346, 11.135)),
        rand2dTo1d(_input, float2(83.155, 52.235))
    );
}
float2 rand2dTo2d(float2 _input)
{
    return float2(
        rand2dTo1d(_input, float2(12.989, 78.233)),
        rand2dTo1d(_input, float2(39.346, 11.135))
    );
}

/// ------------------------------------------------------------------
// Rand1d To
float3 rand1dTo3d(float _input)
{
    return float3(
        rand1dTo1d(_input, 0.546),
        rand1dTo1d(_input, 0.546),
        rand1dTo1d(_input, 0.546)
    );
}
float2 rand1dTo2d(float _input)
{
    return float2(
        rand1dTo1d(_input, 0.546),
        rand1dTo1d(_input, 0.546)
    );
}


// structと class は 同じ
// hlsl に private,publicのようなアクセス修飾子はない
class RandomGenerator3d
{
    float3 seed;
    float3 Generate3d()
    {
        seed = rand3dTo3d(seed);
        return seed;
    }
    float2 Generate2d()
    {
        seed.xy = rand3dTo2d(seed);
        return seed.xy;
    }
    float Generate1d()
    {
        seed.x = rand3dTo1d(seed);
        return seed.x;
    }
    
    float3 Generate3d(float3 _min, float3 _max)
    {
        seed = rand3dTo3d(seed);
        return lerp(_min, _max, seed);
    }
    float2 Generate2d(float2 _min, float2 _max)
    {
        seed.xy = rand3dTo2d(seed);
        return lerp(_min, _max, seed.xy);
    }
    float Generate1d(float _min, float _max)
    {
        seed.x = rand3dTo1d(seed);
        return lerp(_min, _max, seed.x);
    }
};

class RandomGenerator2d
{
    float2 seed;
    float3 Generate3d()
    {
        float3 result = rand2dTo3d(seed);
        seed = result.xy;
        return result;
    }
    float2 Generate2d()
    {
        seed = rand2dTo2d(seed);
        return seed;
    }
    float Generate1d()
    {
        seed.x = rand2dTo1d(seed);
        return seed.x;
    }

    float3 Generate3d(float3 _min, float3 _max)
    {
        float3 result = rand2dTo3d(seed);
        seed = result.xy;
        return lerp(_min, _max, result);
    }
    float2 Generate2d(float2 _min, float2 _max)
    {
        seed = rand2dTo2d(seed);
        return lerp(_min, _max, seed);
    }
    float Generate1d(float _min, float _max)
    {
        seed.x = rand2dTo1d(seed);
        return lerp(_min, _max, seed.x);
    }
    
};

class RandomGenerator1d
{
    float seed;
    float3 Generate3d()
    {
        float3 result = rand1dTo3d(seed);
        seed = result.x;
        return result;
    }
    float2 Generate2d()
    {
        float2 result = rand1dTo2d(seed);
        seed = result.x;
        return result;
    }
    float Generate1d()
    {
        seed = rand1dTo1d(seed);
        return seed;
    }

    float3 Generate3d(float3 _min, float3 _max)
    {
        float3 result = rand1dTo3d(seed);
        seed = result.x;
        return lerp(_min, _max, result);
    }
    float2 Generate2d(float2 _min, float2 _max)
    {
        float2 result = rand1dTo2d(seed);
        seed = result.x;
        return lerp(_min, _max, result);
    }
    float Generate1d(float _min, float _max)
    {
        seed = rand1dTo1d(seed);
        
        return lerp(_min, _max, seed);
    }
};
