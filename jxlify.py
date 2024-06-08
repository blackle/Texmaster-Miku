#!/usr/bin/env python3

from PIL import Image
from jxlpy import JXLImagePlugin
from io import BytesIO
from tqdm import tqdm
import shutil
import imagehash

sizes = []
count = 0
hashes = set()
with open("miku_data.bin", "rb") as data:
	def read_all():
		while True:
			dat = data.read(230454)
			if not dat:
				break
			yield dat

	with open("miku_data_jxl.bin", "wb") as miku_data:
		for bmp in tqdm(read_all()):
			img = Image.open(BytesIO(bmp))
			hs = imagehash.phash(img, hash_size=16)
			if hs in hashes:
				continue
			hashes.add(hs)
			count += 1
			buf = BytesIO()
			img.save(buf, format='jxl', quality=93, effort=6)
			size = buf.getbuffer().nbytes
			sizes.append(size)
			buf.seek(0)
			shutil.copyfileobj(buf, miku_data, length=size)

sizes = ",".join([str(s) for s in sizes])
print(f"const int miku_image_sizes[] = {{{sizes}}};")
print(f"const int miku_image_count = {count};")
print(f"extern const char* _binary_miku_data_jxl_bin_start;")
print(f"extern const char* _binary_miku_data_jxl_bin_end;")
