"""Convert the BLCA logo and compile the ESC panel's decorative reflections."""

from math import exp, sin
from pathlib import Path
import struct

from PIL import Image, ImageDraw, ImageFilter


ADDON = Path(__file__).resolve().parents[1]
OUTPUT = ADDON / "UI/Textures/BLCA"


def write_edds(name, image):
    width, height = image.size
    header = struct.pack(
        "<4s7I11I8I5I", b"DDS ", 124, 0x100F, height, width, width * 4, 0, 1,
        *([0] * 11), 32, 4, int.from_bytes(b"DX10", "little"), 0, 0, 0, 0, 0,
        0x1000, 0, 0, 0, 0,
    )
    (OUTPUT / name).write_bytes(
        header + struct.pack("<5I", 29, 3, 0, 1, 1) + image.tobytes()
    )


def tint(mask, rgb, strength=1):
    layer = Image.new("RGBA", mask.size, rgb)
    layer.putalpha(mask.point(lambda value: round(value * strength)))
    return layer


def logo():
    source = ADDON / "Tools/NewsSources/BLCA_Logo.png"
    with Image.open(source) as image:
        # Preserve every supplied pixel. The UI fits the original aspect ratio.
        return image.convert("RGBA")


def reflections():
    width, height = 512, 1024
    diffuse = Image.new("L", (width, height))
    diffuse.putdata([
        round(18 * exp(-((x - 415) / 160) ** 2 - ((y - 83) / 112) ** 2)
              + 10 * exp(-((x - 472) / 80) ** 2 - ((y - 890) / 173) ** 2))
        for y in range(height)
        for x in range(width)
    ])
    lines = Image.new("L", (width, height))
    draw = ImageDraw.Draw(lines)
    for base, amplitude, phase, opacity in (
        (52, 24, 0.7, 34), (70, 21, 0.8, 23), (94, 31, 0.4, 36),
        (121, 29, 0.2, 21), (865, 31, 2.6, 23), (910, 36, 2.9, 29),
    ):
        points = [
            (x, round(base + amplitude * sin(x / 95 + phase) + 9 * sin(x / 39 + phase)))
            for x in range(width)
        ]
        draw.line(points, fill=opacity, width=3)
    lines = lines.filter(ImageFilter.GaussianBlur(1.8))
    lines.putdata([
        round(value * (index % width / (width - 1)) ** 2)
        for index, value in enumerate(lines.tobytes())
    ])
    return Image.alpha_composite(tint(diffuse, (45, 140, 208)), tint(lines, (111, 195, 255)))


if __name__ == "__main__":
    write_edds("BLCA_PauseBrand.edds", logo())
    write_edds("BLCA_PauseWater.edds", reflections())
