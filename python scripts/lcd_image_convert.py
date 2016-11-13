############################################################################################
# Nokia C100 LCD (132x162) Image Conversion Utility
#
# Ankit Bhatnagar
# November 3 2016
# ankit.bhatnagarindia@gmail.com
#
# USAGE:
#   python lcd.py <image_file_name>
#
# OUTPUTS:
#	output.lcd - final resized image (132x162) in RBG(565) binary format
#	output.jpg - final resized image (132x162) in jpeg format
############################################################################################

#!/usr/bin/python

import os,sys,Image,struct

#create and open output file
output_name = "output.lcd"
image_name = sys.argv[1]
out1=open(output_name,"wb")

print "processing image : " + image_name
v=Image.open(image_name) 			#open image for processing
width = (v.size)[0]					#check if image needs to be rotated
height = (v.size)[1]
rotate_image = False
if(width>height):
	rotate_image = True
print  "Image rotation required : " + str(rotate_image)

img2 = v.copy()
if(rotate_image):				#rotate 90 clockwise if required
	img2 = img2.rotate(-90)						
img2 = img2.resize((132,162), Image.ANTIALIAS)	#resize image to fill LCD
img2.save("output.jpg", "JPEG")

#process image
img2_in = Image.open("output.jpg")
img2_ptr = img2_in.load()
print "Fill image size : " + str(img2_in.size)
print "Scanning image L->R T->B"
pixel_num=0
byte_num=0
pixel_r=0
pixel_g=0
pixel_b=0
lcd_byte_1 = 0
lcd_byte_2 = 0
for i in range((img2_in.size)[1]):
	for j in range ((img2_in.size)[0]):
		#Do pixel RGB processing
		r = (img2_ptr[j,i][0] >> 3) & 0x1f
		g = (img2_ptr[j,i][1] >> 2) & 0x3f
		b = (img2_ptr[j,i][2] >> 3) & 0x1f
		out1.write(struct.pack('H', (r << 11) + (g << 5) + b))
		pixel_num += 1
		byte_num += 2
print "Processed fill image : " + str(pixel_num) + " pixels, " + str(byte_num) + " bytes"
out1.close()	