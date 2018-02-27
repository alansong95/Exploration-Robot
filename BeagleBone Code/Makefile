all: pc bone
pc:

bone:
	arm-linux-gnueabihf-gcc -o Capture_to_UDPs Capture_to_UDPs.c
	arm-linux-gnueabihf-gcc -o Send_UDPs Send_UDPs.c
	arm-linux-gnueabihf-gcc -o Control_TMR_Bone Control_TMR_Bone.c
clean: 
	rm *~ Camera_PC Capture_to_UDPs Send_UDPs Robot_PC