import os
import sys
import ffmpegio
from PIL import Image
import numpy as np
import serial
import getopt

video = None
port = None
image = None
flip = 0xFF

def process_video():
    with ffmpegio.open(video, 'rv', blocksize=1, pix_fmt='gray', s=(128,128), r=3) as fin:
        for frame in fin:
            process_frame(frame)

def process_frame(frame):
    ffmpegio.image.write('./temp.png', frame, overwrite=True)
    col = Image.open('./temp.png')
    gray = col.convert('L')
    bw = np.asarray(gray).copy()
    bw[bw < 128] = 0    # Black
    bw[bw >= 128] = 255 # White
    packet = bytearray()
    packet.append(0xAA) # Magic number
    byte = 0x00
    bit = 0
    for row in bw:
        for pixel in row:
            if(pixel == flip):
                byte = byte | (1 << bit)
            bit += 1
            if(bit == 8):
                if(byte == 0xAA): # Magic number escape
                    byte = 0xAB
                packet.append(byte)
                byte = 0x00
                bit = 0
    port.write(packet)

def process_image():
    col = Image.open(image)
    gray = col.convert('L')
    size = (128, 128)
    gray = gray.resize(size)
    bw = np.asarray(gray).copy()
    bw[bw < 128] = 0    # Black
    bw[bw >= 128] = 255 # White
    packet = bytearray()
    packet.append(0xAA) # Magic number
    byte = 0x00
    bit = 0
    for row in bw:
        for pixel in row:
            if(pixel == flip):
                byte = byte | (1 << bit)
            bit += 1
            if(bit == 8):
                if(byte == 0xAA): # Magic number escape
                    byte = 0xAB
                packet.append(byte)
                byte = 0x00
                bit = 0
    port.write(packet)

if __name__ == '__main__':
    # Remove 1st argument from the
    # list of command line arguments
    argumentList = sys.argv[1:]
 
    # Options
    options = "v:i:p:fh"
    
    # Long options
    long_options = ["video", "image", "port", "flip", "help"]
     
    try:
        # Parsing argument
        arguments, values = getopt.getopt(argumentList, options, long_options)
        
        # checking each argument
        for currentArgument, currentValue in arguments:
    
            if currentArgument in ("-v", "--video"):
                video = currentValue
                
            elif currentArgument in ("-i", "--image"):
                image = currentValue
                
            elif currentArgument in ("-p", "--port"):
                port = serial.Serial(currentValue, 115200)
                
            elif currentArgument in ("-f", "--flip"):
                flip = 0x00
                
            elif currentArgument in ("-h", "--help"):
                print ("Help")
                
    except getopt.error as err:
        # output error, and return with an error code
        print (str(err))

    if(port == None):
        print("Please, set the serial port!")
        sys.exit()

    if(video != None):
        process_video()
        sys.exit()

    if(image != None):
        process_image()
        process_image()
        sys.exit()