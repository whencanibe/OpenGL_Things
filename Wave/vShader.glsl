#version 330

uniform float uTime;
uniform bool uWaving;
in vec4 vPosition;			
in vec4 vColor;

out vec4 color;
  

void main()
{
	float ang = uTime*30/180.0f*3.141592f;
	float ang2 = uTime*360/180.0f*3.141592f;

	float lean_angle = 3.141592f/ 4.0f;
	vec4 pos = vPosition;

	mat4 m1 = mat4(1.0f);			
	m1[1][1] = cos(lean_angle);
	m1[2][1] = -sin(lean_angle);
	m1[1][2] = sin(lean_angle);
	m1[2][2] = cos(lean_angle);
	
	mat4 mz_rotate = mat4(1.0f);
	
	mz_rotate[0][0] = cos(ang);
	mz_rotate[1][0] = -sin(ang);
	mz_rotate[0][1] = sin(ang);
	mz_rotate[1][1] = cos(ang);

    float wave_frequency = 3.0f * 3.141592f * 2.0f;    // 3번 정도 물결치도록 주파수 설정
    float wave_amp = 0.1f;     // 진폭계수
  

    float distance = length(vPosition.xy);
    
    float exp_factor = 1 - exp(2.0 * (1-distance)); // 파동을 감소시키는 지수함수형태
	float wave = sin(wave_frequency*distance - ang2) * wave_amp * exp_factor;

	
	if(uWaving == true)
		if(distance <= 1.0)
			pos.z += wave; 
	
	gl_Position = m1 * mz_rotate * pos;

	gl_Position.w = 1.0f;
	color = vColor;
	
	if(uWaving == true){
		vec4 orange = vec4(2.0, 1.0, 0.0, 1.0);  // 주황색
		vec4 skyBlue = vec4(0.2, 1.0, 2.0, 1.0);  // 하늘색

		if(distance <= 1.0){
			if (wave < 0.0) {
				color = mix(vColor, orange, abs(wave));  
			
				color.w = 1.0f;
			} else if (wave > 0.0){
				color = mix(vColor, skyBlue, abs(wave));  
			
				color.w = 1.0f;
			}
		}
		
	}
	


	
}