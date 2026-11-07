#version 330 core

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

out vec4 FragColor;
vec2 intersectAABB(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax) {
    vec3 tMin = (boxMin - rayOrigin) / rayDir;
    vec3 tMax = (boxMax - rayOrigin) / rayDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
};

//	Simplex 3D Noise 
//	by Ian McEwan, Stefan Gustavson (https://github.com/stegu/webgl-noise)
//
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}

float snoise(vec3 v){ 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //  x0 = x0 - 0. + 0.0 * C 
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;

// Permutations
  i = mod(i, 289.0 ); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients
// ( N*N points uniformly over a square, mapped onto an octahedron.)
  float n_ = 1.0/7.0; // N=7
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
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
		float density = clamp(snoise(samplePoint ),0.0,1.0); // Escalar para ajustar la frecuencia del ruido
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
		float density = clamp(snoise(samplePoint ),0.0,1.0); // Escalar para ajustar la frecuencia del ruido
		pos+=u_step_size;
	}
	
    transmittance = exp(-accumulatedDensity*u_absortion_coefficient);
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




