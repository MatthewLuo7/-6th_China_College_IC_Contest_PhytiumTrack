Brief Introduction
========
This repository contains our first-prize work in the 6th China College IC Contest. We designed a high-performance JPEG encoder and decoder using C++, and implemented it on FT2000/4. We leveraged AAN algorithm, Neon instructions, and matrix transformation to accelerate DCT and iDCT, and we also optimized the coding and decoding with Huffman tree and look-up table. To add more practical value, we deployed a light DNN through MNN to realize object-detection. Furthermore, we explored the idea, learning in the frequency domain, which directly predicts results through image's DCT coefficients instead of inputing pixels as traditional DNNs do and thus expedites the inference.

Key Words
========
* JPEG Encoder and Decoder; 
* AAN-based DCT; 
* Neon Instructions;
* Object-detection;
* Learning in the Frequency Domain.
* MNN-based Deployment.

Required Devices
========
* FT2000/4 Development Board;
* Camera, Mouse, Keyboard, Displayer and etc.

Required Libraries
======== 
* Refer Makefile

Deployment
========
* Generate xFLOW by running ./make
* Run ./xFLOW
