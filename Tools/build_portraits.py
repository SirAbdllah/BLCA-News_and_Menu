"""Convert the generated transparent PNG assets into Reforger RGBA DDS textures."""
from pathlib import Path
from PIL import Image
import struct,re
root=Path(__file__).resolve().parents[1]
for name,guid in [('Army','CB620701458A4D31'),('USSR','CB620701458A4D32')]:
 src=root/'Tools/PortraitSources'/('ArmyVeteran.png' if name=='Army' else 'USSR.png')
 image=Image.open(src).convert('RGBA')
 assert image.getchannel('A').getextrema()==(0,255), 'Expected transparent cutout'
 w,h=image.size
 header=struct.pack('<4s7I11I8I5I',b'DDS ',124,0x100F,h,w,w*4,0,1,*([0]*11),32,4,int.from_bytes(b'DX10','little'),0,0,0,0,0,0x1000,0,0,0,0)
 relative=f'UI/Textures/BN/{name}Portrait.edds'
 output=root/relative
 output.write_bytes(header+struct.pack('<5I',29,3,0,1,3)+image.tobytes())
 meta=(root/'UI/Textures/BN/Glyphs.edds.meta').read_text(encoding='utf-8-sig')
 meta=re.sub(r'Name "[^"]+"',f'Name "{{{guid}}}{relative}"',meta,count=1)
 output.with_name(output.name+'.meta').write_text(meta,encoding='utf-8')
 decoded=Image.open(output);decoded.load()
 assert decoded.getchannel('A').getextrema()==(0,255)
 print(name,decoded.size,'RGBA texture verified')
