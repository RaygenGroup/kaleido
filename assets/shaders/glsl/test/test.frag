#version 460 core
#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable
out vec4 out_color;
  
in Data
{ 
	vec3 pos;  
	vec3 normal;
	vec4 tangent;
	vec3 bitangent;
	vec2 textCoord0;
	vec2 textCoord1;
	mat3 TBN;
} dataIn;

uniform int mode;

uniform vec3 viewPos;

uniform vec4 baseColorFactor;
uniform vec3 emissiveFactor;
uniform float metallicFactor;
uniform float roughnessFactor;
uniform float normalScale;
uniform float occlusionStrength;
uniform int alphaMode;
uniform float alphaCutoff;
uniform bool doubleSided;

//mandatory
layout(bindless_sampler) uniform sampler2D baseColorSampler;
layout(bindless_sampler) uniform sampler2D occlusionMetallicRoughnessSampler;
layout(bindless_sampler) uniform sampler2D emissiveSampler;
//optional
uniform uint64_t normalSampler;

#define M_1_PIf 0.318309886183790671538f
#define M_PIf 3.14159265358979323846f

	// TODO render to FBO, sample depth texture, show hdr skymap at far
	/*vec3 dir = normalize(FragPos - viewPos);

	//float theta = atan(dir.x, dir.z);
	//float phi = M_PIf * 0.5f - acos(dir.y);
	//float u = (theta + M_PIf) * (0.5f * M_1_PIf);
	//float v = 0.5f * (1.0f + sin(phi));
	//if(depth)
	//out_color = vec4(vec3(texture(skyHDRSampler, vec2(u,v))), 1.0);*/

// very basic unoptimized surface test shader
void main()
{
	const vec4 baseColor = texture(baseColorSampler, dataIn.textCoord0);
	const vec4 occlusionMetallicRoughness = texture(occlusionMetallicRoughnessSampler, dataIn.textCoord0);
	const vec4 emissive = texture(emissiveSampler, dataIn.textCoord0);
	
	
	//out_color = occlusionMetallicRoughness;
	//return;
	
	// missing
	out_color = vec4(1.f, 0.f, 1.f, 1.f);
	
	switch (mode)
	{
		case 0: // base color map
			out_color = baseColor;
			break;
			
		case 1: // base color factor
			out_color = baseColorFactor;
			break;
			
		case 2: // base color final
			out_color = baseColor * baseColorFactor;
			break;
			
		case 3: // metallic map
			out_color = vec4(occlusionMetallicRoughness.bbb, 1.0);
			break;
			
		case 4: // metallic factor
			out_color = vec4(metallicFactor);
			break;
			
		case 5: // metallic final
			out_color = vec4(occlusionMetallicRoughness.bbb, 1.0) * metallicFactor;
			break;
			
		case 6: // roughness map
			out_color = vec4(occlusionMetallicRoughness.ggg, 1.0);
			break;
			
		case 7: // roughness factor
			out_color = vec4(roughnessFactor);
			break;
			
		case 8: // roughness final
			out_color = vec4(occlusionMetallicRoughness.ggg, 1.0) * roughnessFactor;
			break;
			
		case 9: // normal
			out_color = vec4(dataIn.normal, 1.0);
			break;
			
		case 10: // normal scale
			out_color = vec4(normalScale);
			break;
			
		case 11: // normal map
			
			// normal mapping
			if(normalSampler>0)
			{
				out_color = texture(sampler2D(normalSampler), dataIn.textCoord0);
			}
		
			break;
			
		case 12: // normal final
		
			if(normalSampler>0)
			{
				vec3 sampleNormal = texture(sampler2D(normalSampler), dataIn.textCoord0).rgb;
				vec3 scaledNormal = normalize((sampleNormal * 2.0 - 1.0) * vec3(normalScale, normalScale, 1.0));
				out_color = vec4(normalize(dataIn.TBN * scaledNormal), 1);
			}
			else
			{
				out_color = vec4(dataIn.normal, 1.0);
			}
			break;
			
		case 13: // tangent
			out_color = dataIn.tangent;
			break;
			
		case 14: // tangent handedness
			out_color = vec4(dataIn.tangent.a);
			break;
			
		case 15: // bitangent
			out_color = vec4(dataIn.bitangent, 1.0);
			break;
			
		case 16: // occlusion map
			out_color = vec4(occlusionMetallicRoughness.rrr, 1.0);
			break;
			
		case 17: // occlusion strength
			out_color = vec4(occlusionStrength);
			break;
			
		case 18: // occlusion final
			out_color = mix(baseColor, baseColor * occlusionMetallicRoughness.rrrr, occlusionStrength);
			break;
			
		case 19: // emissive map
			out_color = vec4(emissive.rgb, 1.0);
			break;
			
		case 20: // emissive factor
			out_color = vec4(emissiveFactor, 1.0);
			break;
			
		case 21: // emissive final
			out_color = vec4(emissive.rgb*emissiveFactor, 1.0);
			break;
			
		case 22: // opacity map
			out_color = vec4(baseColor.a);
			break;
			
		case 23: // opacity factor
			out_color = vec4(baseColorFactor.a);
			break;
			
		case 24: // opacity final TODO: alpha blending

			break;
			
		case 25: // texture coordinate 0
			out_color = vec4(dataIn.textCoord0, 0.0, 1.0);
			break;
			
		case 26: // texture coordinate 1
			out_color = vec4(dataIn.textCoord1, 0.0, 1.0);
			break;
			
		case 27: // alpha mode
			switch (alphaMode)
			{
			// opaque
			case 0:
				out_color = vec4(1.f, 0.f, 0.f, 1.f);
				break;
			// mask
			case 1:
				out_color = vec4(0.f, 1.f, 0.f, 1.f);
				break;
			// blend
			case 2:
				out_color = vec4(0.f, 0.f, 1.f, 1.f);
				break;
			}
			break;
			
		case 28: // alpha cutoff
			out_color = vec4(alphaCutoff);
			break;
	
		case 29: // alpha mask
			out_color = vec4(0.f, 0.f, 0.f, 1.f);
			if(alphaMode == 1)
			{
				// transparent
				if(baseColor.a < alphaCutoff)
				{
					out_color = vec4(1.f, 1.f, 1.f, 1.f);
				}
			}
			break;
			
		case 30: // double sidedness
			if (doubleSided)
			{
				out_color = vec4(1.f, 1.f, 1.f, 1.f);
			}
			else
			{
				out_color = vec4(0.f, 0.f, 0.f, 1.f);
			}
			break;
	}
}