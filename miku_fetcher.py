#!/usr/bin/env python3
import requests
import sys
from PIL import Image
from io import BytesIO
from tqdm import tqdm
import shutil
import os
import xml.etree.ElementTree as ET

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def image_to_bmp(url):
	response = requests.get(url)
	if response.status_code != 200:
		eprint("failed to get", url)
		return None
	image = Image.open(BytesIO(response.content))
	if image.mode != "RGB":
		return None
	resized_image = image.resize((320, 240))
	buf = BytesIO()
	resized_image.save(buf, format='BMP')
	return buf

def get_images_page(width, height, page):
	imgs = []
	url = """https://safebooru.org/index.php?page=dapi"""
	url += "&s=post&q=index"
	url += "&limit=100"
	url += f"&pid={page}"
	url += f"&tags=hatsune_miku+solo+-animated+-cosplay+sort%3ascore+score%3a>-1+width%3a{width}+height%3a{height}"
	response = requests.get(url)
	if response.status_code == 200:
		root = ET.fromstring(response.content)
		for post in tqdm(root):
			bmp = image_to_bmp(post.attrib['file_url'])
			if bmp:
				imgs.append(bmp)
	else:
		eprint(f"Failed to fetch data: {response.status_code}")
	return imgs

def get_images(width, height):
	imgs = []
	for k in range(5):
		myimgs = get_images_page(width, height, k)
		if len(myimgs) == 0:
			break
		imgs += myimgs
	return imgs

imgs = []
for i in range(8):
	scale = i+1
	imgs += get_images(256*scale, 192*scale)
	imgs += get_images(320*scale, 240*scale)
	imgs += get_images(384*scale, 288*scale)
	imgs += get_images(400*scale, 300*scale)

with open("miku_data.bin", "wb") as miku_data:
	last_size = None
	col = 1
	img_count = 0
	for img in tqdm(imgs):
		size = img.getbuffer().nbytes
		img.seek(0)
		if last_size != size and last_size is not None:
			eprint("had to skip an image! size=",size)
			continue
		img_count+=1
		shutil.copyfileobj(img, miku_data, length=size)
		last_size = size

print(f"unsigned int miku_img_count = {img_count};")
print(f"unsigned int miku_img_size = {last_size};")
print(f"extern const char* _binary_miku_data_bin_start;")
print(f"extern const char* _binary_miku_data_bin_end;")