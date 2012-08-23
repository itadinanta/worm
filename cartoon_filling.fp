!!ARBfp1.0
	PARAM tresholds = {0.2, 0.3, 0.80, 100}; # tresholds
	PARAM midLevel =  {0.6, 0.5, 0.95, 1}; # 4 Lighting values for this shader
	PARAM treshSpec = 1.00;
	PARAM reflShade = 0.20;
	TEMP tresholds1;
	TEMP grayLevel, cColor;
	TEMP cartoon1, cartoon2;
	TEMP texColor, reflColor;
	OUTPUT oColor = result.color;

	SWZ tresholds1, tresholds, -w,x,y,z;

# texcoord1.x contains interpolated diffuse component
# texcoord1.y contains specular approximate component

	MOV grayLevel, fragment.texcoord[1];

# "cartoon" effect by mapping linear on the 4-step stair defined by thresholds

	SLT cartoon2, grayLevel.x, tresholds;	# finds which upper boundary has not been reached
	SGE cartoon1, grayLevel.x, tresholds1;  # finds which lower boundary has been overrun
	MUL cartoon2, cartoon1, cartoon2;	# only the selected steps gains 1.
	DP4 grayLevel.x, cartoon2, midLevel; 	# for each level range, lookup for its actual value

# shading

	MOV cColor, fragment.color;

# Modulate textures here

	MOV texColor, fragment.texcoord[0];
	MOV texColor.zw, 0.0;

# ADD reflection effect
# is it really that easy?

	TEX reflColor, fragment.texcoord[2], texture[2], CUBE; 

# shading

#	ADD reflColor, reflColor, -cColor;
#	MAD cColor, reflColor, reflShade, cColor;

# blend decal

	TEX texColor, texColor, texture[0], 2D;
	MAD cColor, cColor, -texColor.a, cColor;
	MAD cColor.rgb, texColor, texColor.a, cColor;

# saturation for specular (highlight only)

#	SGE grayLevel.y, grayLevel.y, treshSpec;
#	SGE reflColor, reflColor.y, treshSpec;

# shading (use white only for specular)

#	MAD cColor, cColor, grayLevel.x, grayLevel.y;
	MAD cColor, cColor, grayLevel.x, reflColor;

# output
	
	MOV oColor.rgb, cColor;

# restores alpha
	
	MOV cColor.a, fragment.color.a;

END
