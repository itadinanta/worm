!!ARBfp1.0
	PARAM tresholds = {0.2, 0.3, 0.80, 100}; # tresholds
	PARAM midLevel =  {0.8, 0.9, 0.95, 1}; # 4 Lighting values for this shader
	PARAM treshSpec = 1.00;
	PARAM reflShade = 0.20;
	TEMP tresholds1;
	TEMP grayLevel, cColor;
	TEMP cartoon1, cartoon2;
	TEMP texColor0, texColor1, texColor2, texColor3, reflColor, fColor;
	OUTPUT oColor = result.color;

	SWZ tresholds1, tresholds, -w,x,y,z;

# shading

	MOV cColor, fragment.color;

# use texcoord0/texture[0] as the master texture

	TEX texColor0, fragment.texcoord[0], texture[0], 2D;

# modulates texture by color (bah?)

	MUL cColor, cColor, texColor0;

# fragment.color.a must contain interpolated precalculated diffuse component to use as a shadowmap

	MOV grayLevel, cColor.a;

# "cartoon" effect by mapping linear on the 4-step stair defined by thresholds

	SLT cartoon2, grayLevel.x, tresholds;	# finds which upper boundary has not been reached
	SGE cartoon1, grayLevel.x, tresholds1;  # finds which lower boundary has been overrun
	MUL cartoon2, cartoon1, cartoon2;	# only the selected steps gains 1.
	DP4 grayLevel.x, cartoon2, midLevel; 	# for each level range, lookup for its actual value

# Modulate 3 textures here, using texcoord[0] (master texture) lookup as weighs

	TEX texColor1, fragment.texcoord[1], texture[1], 2D;
	TEX texColor2, fragment.texcoord[2], texture[2], 2D;
	TEX texColor3, fragment.texcoord[3], texture[3], 2D;

	MUL fColor, texColor1, texColor0.r;
	MAD fColor, texColor2, texColor0.g, fColor;
	MAD fColor, texColor3, texColor0.b, fColor;

# add cartoon shading

	MUL fColor, fColor, grayLevel.x;

# restores alpha

	MOV fColor.a, 1.0;

# output
	
	MOV oColor, fColor;

END
