!!ARBfp1.0
#
#	Simple pixel shader for solid outline
#	(C) Nicola Orrù 2004
#	keep it simple!
#
	PARAM black = {0,0,0,1};
	OUTPUT oColor = result.color;
	OUTPUT oDepth = result.depth;
#
#	leave it as it is
#
#	MOV oColor, fragment.color;
	MOV oColor.a, fragment.color;
#
# 	depth offset to bring out lines
#
	SUB oDepth.z, fragment.position.z, 0.00006;	
END
