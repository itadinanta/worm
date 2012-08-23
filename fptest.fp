!!ARBfp1.0
PARAM tresholds1 = {-100, 0.1, 0.7, 0.9};
PARAM tresholds2 = {0.1, 0.7, 0.9, 100};
PARAM midLevel = {0.1, 0.7, 0.9, 1.0}; # 4 Lighting values for this shader
PARAM treshSpec = 0.80;
TEMP grayLevel, cColor;
TEMP cartoon1, cartoon2;
OUTPUT oColor = result.color;

# texcoord1.x contains interpolated diffuse component
# texcoord1.y contains specular approximate component

MOV grayLevel, fragment.texcoord[1];

# "cartoon" effect by mapping linear on the 4-step stair defined by thresholds
SGE cartoon1, grayLevel.x, tresholds1;  # finds which lower boundary has been overrun
SLT cartoon2, grayLevel.x, tresholds2;	# finds which upper boundary has not been reached
MUL cartoon2, cartoon1, cartoon2;	# only the selected steps gains 1.
DP4 grayLevel.x, cartoon2, midLevel; 	# for each level range, lookup for its actual value

# saturation for specular (highlight only)
SGE grayLevel.y, grayLevel.y, treshSpec;

# shading (use white only for specular)
MAD cColor, fragment.color, grayLevel.x, grayLevel.y;

# restores alpha
MOV cColor.a, fragment.color.a;

# Modulate textures here
MOV oColor, cColor;
# MOV oColor, grayLevel.x;
END
