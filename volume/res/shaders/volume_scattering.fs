#version 330 core
// Lab 4: Lab 3 volume shader + density from a VDB 3D texture + in-scattering from a light

in vec3 v_position;
in vec3 v_world_position;
in vec3 v_normal;

uniform vec3 u_boxmin;
uniform vec3 u_boxmax;
uniform vec3 u_camera_position;
uniform vec4 u_background_color;
uniform vec3 u_emitted_color;
uniform int u_task;
uniform float u_absortion_coefficient;
uniform float u_step_size;
uniform float u_noise_factor;

// Lab 4 Task 1: density source (0 = 3D noise, 1 = VDB texture) and world -> texture mapping
uniform int u_density_source;
uniform sampler3D u_volume_texture;
uniform mat4 u_inverse_model;
uniform vec3 u_local_boxmin;
uniform vec3 u_local_boxmax;

out vec4 FragColor;
vec2 intersectAABB(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax) {
    vec3 tMin = (boxMin - rayOrigin) / rayDir;
    vec3 tMax = (boxMax - rayOrigin) / rayDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
}

//	Simplex 4D Noise 
//	by Ian McEwan, Stefan Gustavson (https://github.com/stegu/webgl-noise)
//
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
float permute(float x){return floor(mod(((x*34.0)+1.0)*x, 289.0));}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}
float taylorInvSqrt(float r){return 1.79284291400159 - 0.85373472095314 * r;}

vec4 grad4(float j, vec4 ip){
  const vec4 ones = vec4(1.0, 1.0, 1.0, -1.0);
  vec4 p,s;

  p.xyz = floor( fract (vec3(j) * ip.xyz) * 7.0) * ip.z - 1.0;
  p.w = 1.5 - dot(abs(p.xyz), ones.xyz);
  s = vec4(lessThan(p, vec4(0.0)));
  p.xyz = p.xyz + (s.xyz*2.0 - 1.0) * s.www; 

  return p;
}

float snoise(vec4 v){
  const vec2  C = vec2( 0.138196601125010504,  // (5 - sqrt(5))/20  G4
                        0.309016994374947451); // (sqrt(5) - 1)/4   F4
// First corner
  vec4 i  = floor(v + dot(v, C.yyyy) );
  vec4 x0 = v -   i + dot(i, C.xxxx);

// Other corners

// Rank sorting originally contributed by Bill Licea-Kane, AMD (formerly ATI)
  vec4 i0;

  vec3 isX = step( x0.yzw, x0.xxx );
  vec3 isYZ = step( x0.zww, x0.yyz );
//  i0.x = dot( isX, vec3( 1.0 ) );
  i0.x = isX.x + isX.y + isX.z;
  i0.yzw = 1.0 - isX;

//  i0.y += dot( isYZ.xy, vec2( 1.0 ) );
  i0.y += isYZ.x + isYZ.y;
  i0.zw += 1.0 - isYZ.xy;

  i0.z += isYZ.z;
  i0.w += 1.0 - isYZ.z;

  // i0 now contains the unique values 0,1,2,3 in each channel
  vec4 i3 = clamp( i0, 0.0, 1.0 );
  vec4 i2 = clamp( i0-1.0, 0.0, 1.0 );
  vec4 i1 = clamp( i0-2.0, 0.0, 1.0 );

  //  x0 = x0 - 0.0 + 0.0 * C 
  vec4 x1 = x0 - i1 + 1.0 * C.xxxx;
  vec4 x2 = x0 - i2 + 2.0 * C.xxxx;
  vec4 x3 = x0 - i3 + 3.0 * C.xxxx;
  vec4 x4 = x0 - 1.0 + 4.0 * C.xxxx;

// Permutations
  i = mod(i, 289.0); 
  float j0 = permute( permute( permute( permute(i.w) + i.z) + i.y) + i.x);
  vec4 j1 = permute( permute( permute( permute (
             i.w + vec4(i1.w, i2.w, i3.w, 1.0 ))
           + i.z + vec4(i1.z, i2.z, i3.z, 1.0 ))
           + i.y + vec4(i1.y, i2.y, i3.y, 1.0 ))
           + i.x + vec4(i1.x, i2.x, i3.x, 1.0 ));
// Gradients
// ( 7*7*6 points uniformly over a cube, mapped onto a 4-octahedron.)
// 7*7*6 = 294, which is close to the ring size 17*17 = 289.

  vec4 ip = vec4(1.0/294.0, 1.0/49.0, 1.0/7.0, 0.0) ;

  vec4 p0 = grad4(j0,   ip);
  vec4 p1 = grad4(j1.x, ip);
  vec4 p2 = grad4(j1.y, ip);
  vec4 p3 = grad4(j1.z, ip);
  vec4 p4 = grad4(j1.w, ip);

// Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;
  p4 *= taylorInvSqrt(dot(p4,p4));

// Mix contributions from the five corners
  vec3 m0 = max(0.6 - vec3(dot(x0,x0), dot(x1,x1), dot(x2,x2)), 0.0);
  vec2 m1 = max(0.6 - vec2(dot(x3,x3), dot(x4,x4)            ), 0.0);
  m0 = m0 * m0;
  m1 = m1 * m1;
  return 49.0 * ( dot(m0*m0, vec3( dot( p0, x0 ), dot( p1, x1 ), dot( p2, x2 )))
               + dot(m1*m1, vec2( dot( p3, x3 ), dot( p4, x4 ) ) ) ) ;

}

// Density at a world-space point, in [0, 1]
float sampleDensity(vec3 worldPos) {
	if (u_density_source == 1) {
		// world space -> the cube's local space -> texture coordinates in [0, 1]^3
		vec3 localPos = (u_inverse_model * vec4(worldPos, 1.0)).xyz;
		vec3 uvw = (localPos - u_local_boxmin) / (u_local_boxmax - u_local_boxmin);
		return texture(u_volume_texture, uvw).r;
	}
	return clamp(snoise(vec4(worldPos, u_noise_factor)), 0.0, 1.0);
}

vec4 computeColorHomogeneous(vec3 rayOrigin, vec3 rayDir, float tNear, float tFar) {
	float ta = max(0.0, tNear);
	float tb = tFar;

	float distanceInside = max(0.0, tb - ta);

	// 1. Calcular espesor óptico y transmitancia
	float opticalThickness = distanceInside * u_absortion_coefficient;
	float transmittance = exp(-opticalThickness);

	// 2. Mezclar el color de fondo con la transmitancia
	vec3 finalColor = u_background_color.rgb * transmittance;
	float finalAlpha = u_background_color.a; // O puedes manejar la opacidad global del volumen si lo pide tu práctica

	return vec4(finalColor, finalAlpha);
}
vec4 computeColorEmissionHeterogeneous(vec3 rayOrigin, vec3 rayDir, float tNear, float tFar) {
	float pos=tNear +u_step_size/2; // Iniciar un poco después del primer punto de intersección
	float accumulatedDensity=0.0;
    vec3 accumulatedEmitedLight = vec3(0.0);
	float transmittance=1.0;
	while(pos<=tFar ) {
		vec3 samplePoint = rayOrigin + pos * rayDir;
		float density = sampleDensity(samplePoint); // Escalar para ajustar la frecuencia del ruido
		float mu_a=density*u_absortion_coefficient;
		float stepDensity = mu_a * u_step_size;
		float transmittanceToPoint = exp(-stepDensity);
		accumulatedEmitedLight += mu_a * u_emitted_color * transmittanceToPoint * u_step_size;
		accumulatedDensity += stepDensity;
		pos+=u_step_size;
	}
	
		transmittance = exp(-accumulatedDensity);
	vec3 finalColor = u_background_color.rgb * transmittance+accumulatedEmitedLight;
	float finalAlpha = u_background_color.a; // O puedes manejar la opacidad global del volumen si lo pide tu práctica

	return vec4(finalColor, finalAlpha);
}
vec4 computeColorHeterogeneous(vec3 rayOrigin, vec3 rayDir, float tNear, float tFar) {
	float pos=tNear +u_step_size/2; // Iniciar un poco después del primer punto de intersección
	float accumulatedDensity=0.0;
	float transmittance=1.0;
	while(pos<=tFar ) {
		vec3 samplePoint = rayOrigin + pos * rayDir;
		float density = sampleDensity(samplePoint); // Escalar para ajustar la frecuencia del ruido
		accumulatedDensity+=density*u_absortion_coefficient*u_step_size;
		pos+=u_step_size;
	}
	
    transmittance = exp(-accumulatedDensity);
	vec3 finalColor = u_background_color.rgb * transmittance;
	float finalAlpha = u_background_color.a; // O puedes manejar la opacidad global del volumen si lo pide tu práctica

	return vec4(finalColor, finalAlpha);
}
void main() {
    vec3 rayOrigin = u_camera_position;
    vec3 rayDir = normalize(v_world_position - u_camera_position);

	vec2 t = intersectAABB(rayOrigin, rayDir, u_boxmin, u_boxmax);
	if(t.x > t.y || t.y < 0.0) {
		// No intersection, return background color
		FragColor = u_background_color;
		return;
	}
	if(u_task == 1) {
		FragColor = computeColorHomogeneous(rayOrigin, rayDir, t.x, t.y);
	} else if(u_task == 2) {
		FragColor = computeColorHeterogeneous(rayOrigin, rayDir, t.x, t.y);
	}
	else{
		FragColor = computeColorEmissionHeterogeneous(rayOrigin, rayDir, t.x, t.y);
	}
}




