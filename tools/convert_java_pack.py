"""Convert Java 1.12 block/item textures into a native resource overlay.

Pillow required. This creates atlas resources, not a licensed DLC .pck container.
Unsupported assets/animations are reported; source archives are never extracted.
"""
from pathlib import Path
import argparse, io, json, re, tempfile, zipfile, shutil
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
BASE=ROOT/'Minecraft.Client/Common/res/TitleUpdate/res'
ALIASES={'workbench':'crafting_table','stoneslab':'stone_slab','leaves_oak':'leaves_oak','flower_rose':'flower_poppy','furnace_front':'furnace_front_off','inverted_daylight_detector':'daylight_detector_inverted_top','packed_ice':'ice_packed'}
def java_name(name):
    if name in ALIASES:return ALIASES[name]
    for old,new in ALIASES.items():
        if name.startswith(old+'_'):return new+name[len(old):]
    match=re.fullmatch(r'(helmet|chestplate|leggings|boots|sword|shovel|pickaxe|axe|hoe)(Cloth|Chain|Iron|Diamond|Gold|Wood|Stone)',name)
    if match:return {'Cloth':'leather','Chain':'chainmail','Iron':'iron','Diamond':'diamond','Gold':'golden','Wood':'wooden','Stone':'stone'}[match[2]]+'_'+match[1]
    return re.sub(r'([a-z])([A-Z])',r'\1_\2',name).lower()
def slots(source):
    code=source[source.index('if(iconType != Icon::TYPE_TERRAIN)'):]
    # The native terrain branch begins after all item clock/compass setup.
    split=code.index('ADD_ICON(0,',code.index('L"compassP3"'))
    for group,section in [('items',code[:split]),('terrain',code[split:])]:
        section=re.sub(r'//[^\n]*','',section)
        for m in re.finditer(r'ADD_ICON(?:_WITH_NAME)?\(\s*(\d+)\s*,\s*(\d+)\s*,\s*L"([^"]+)"(?:\s*,\s*L"([^"]+)")?\s*\)',section):
            yield group,int(m[1]),int(m[2]),m[4] or m[3]
def convert(archive,output,base=BASE,source=None):
    output=Path(output)
    if output.exists():raise ValueError('Output must not exist')
    source=source or (ROOT/'Minecraft.Client/PreStitchedTextureMap.cpp').read_text()
    report={'converted':[],'fallback':[],'animated_first_frame':[],'unconverted_assets':[], 'limitations':['Block/item atlas overlay only; not a selectable DLC package.','Entity models, GUI, sounds, custom models and animation timing are not converted.']}
    with zipfile.ZipFile(archive) as z:
        names=z.namelist()
        if len(names)!=len(set(names)):raise ValueError('Duplicate ZIP paths')
        if sum(i.file_size for i in z.infolist())>512*1024*1024:raise ValueError('Pack exceeds 512 MiB uncompressed limit')
        for name in names:
            if name.startswith('/') or '\\' in name or '..' in name.split('/'):raise ValueError('Unsafe ZIP path')
        if 'pack.mcmeta' not in names:raise ValueError('Expected pack.mcmeta at ZIP root')
        metadata=json.loads(z.read('pack.mcmeta'))
        if metadata.get('pack',{}).get('pack_format')!=3:raise ValueError('Expected Java 1.11–1.12.2 pack_format 3')
        used=set();atlases={}
        for group in ['terrain','items']:
            with Image.open(Path(base)/(group+'.png')) as im:atlases[group]=im.convert('RGBA')
            im=atlases[group]
            if im.width%16 or im.height% (im.width//16):raise ValueError('Invalid native base atlas dimensions')
        for group,row,col,native in slots(source):
            atlas=atlases[group];size=atlas.width//16
            if col>=16 or (row+1)*size>atlas.height:raise ValueError('Native slot outside base atlas')
            name=java_name(native);rotation=0
            m=re.fullmatch(r'(.+)_r([0-3])',name)
            if m:name=m[1];rotation=int(m[2])
            key='assets/minecraft/textures/'+('blocks/' if group=='terrain' else 'items/')+name+'.png'
            label=group+':'+native
            if key not in names:report['fallback'].append(label);continue
            if z.getinfo(key).file_size>16*1024*1024:raise ValueError('Oversized texture')
            with Image.open(io.BytesIO(z.read(key))) as image:
                if image.width>4096 or image.height>65536 or image.width<1 or image.height<image.width or image.height%image.width:raise ValueError('Unsupported sprite dimensions: '+key)
                if image.height!=image.width:report['animated_first_frame'].append(key)
                sprite=image.convert('RGBA').crop((0,0,image.width,image.width))
            if rotation:sprite=sprite.rotate(-90*rotation)
            atlas.paste(sprite.resize((size,size),Image.Resampling.NEAREST),(col*size,row*size))
            used.add(key);report['converted'].append(label)
        report['unconverted_assets']=sorted(n for n in names if n.startswith('assets/') and not n.endswith('/') and n not in used)
        output.parent.mkdir(parents=True,exist_ok=True)
        with tempfile.TemporaryDirectory(dir=output.parent) as temporary:
            staging=Path(temporary)/'overlay';(staging/'res').mkdir(parents=True)
            for group,atlas in atlases.items():atlas.save(staging/'res'/(group+'.png'))
            (staging/'conversion-report.json').write_text(json.dumps(report,indent=2)+'\n')
            (staging/'README.txt').write_text('Native atlas resource overlay, NOT a selectable pack.\nApply through a compatible native pack adapter. Inspect conversion-report.json.\nMissing mappings retain the supplied native base art. Animations use their first frame.\n')
            staging.rename(output)
    return report
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('archive');p.add_argument('output');p.add_argument('--base-res',type=Path,default=BASE);args=p.parse_args()
    r=convert(args.archive,args.output,args.base_res);print('Converted:',len(r['converted']),'Fallback:',len(r['fallback']),'See conversion-report.json')
