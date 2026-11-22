#!/usr/bin/env python3
"""Encode opening_film.c's real-ROM video without changing its playback speed.

Usage: python3 tools/encode_opening_film.py CAPTURE_DIRECTORY [OUTPUT_STEM]

The MP4 preserves the exact GBA video rate by default. The optional 20/30 FPS
MP4 and 20 FPS GIF sample those original timestamps; they do not reinterpret
the input as a round-number frame rate. Frames are scaled with nearest-neighbor
filtering. Both exports are silent; native PPMs remain the pixel-exact evidence.
Requires only Python's standard library, ffmpeg and ffprobe.
"""
import argparse
import csv
from fractions import Fraction
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import zlib


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def probe(path):
    return json.loads(subprocess.check_output([
        "ffprobe", "-v", "error", "-select_streams", "v:0",
        "-show_entries",
        "stream=codec_name,width,height,r_frame_rate,avg_frame_rate,duration,nb_frames:format=duration",
        "-of", "json", str(path),
    ], text=True))


def verify_export(path, rate, duration, width, height, expected_frames=None):
    result = probe(path)
    streams = result.get("streams", [])
    if len(streams) != 1:
        raise RuntimeError(f"{path}: expected one encoded video stream")
    stream = streams[0]
    if (stream["width"] != width or stream["height"] != height or
            Fraction(stream["avg_frame_rate"]) != rate):
        raise RuntimeError(f"{path}: dimensions or playback cadence changed")
    tolerance = 1e-5 if expected_frames is not None else float(1 / rate) + 1e-5
    if abs(float(stream["duration"]) - duration) > tolerance:
        raise RuntimeError(f"{path}: encoded duration does not match the source timeline")
    if expected_frames is not None and int(stream["nb_frames"]) != expected_frames:
        raise RuntimeError(f"{path}: native video frames were lost or duplicated")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture_directory", type=Path)
    parser.add_argument("output_stem", type=Path, nargs="?")
    parser.add_argument("--fps", choices=("native", "20", "30"), default="native",
                        help="MP4 video rate; default retains every GBA interval")
    parser.add_argument("--scale", type=int, choices=range(1, 7), default=3,
                        help="integer nearest-neighbor display scale (default: 3)")
    parser.add_argument("--skip-gif", action="store_true")
    args = parser.parse_args()
    for executable in ("ffmpeg", "ffprobe"):
        if not shutil.which(executable):
            parser.error(f"{executable} is required")
    directory = args.capture_directory.resolve()
    meta_path = directory / "capture.json"
    metadata = json.loads(meta_path.read_text())
    if (metadata.get("format_version") != 1 or
            metadata.get("source") != "mGBA completed GBA video scanout" or
            metadata.get("frame_pattern") != "frame-%06d.ppm" or
            metadata.get("width") != 240 or metadata.get("height") != 160):
        parser.error("capture metadata is not a native opening_film.c capture")
    count = metadata["frame_count"]
    rate = Fraction(metadata["cpu_frequency_hz"], metadata["cycles_per_video_frame"])
    if count < 1 or rate <= 0:
        parser.error("invalid source frame count or video cadence")
    duration = float(count / rate)
    if abs(duration - metadata["duration_seconds"]) > 1e-9:
        parser.error("source duration does not match its video-frame cadence")
    with (directory / "frames.csv").open(newline="") as stream:
        timeline = list(csv.DictReader(stream))
    if len(timeline) != count:
        parser.error("source timeline is incomplete")
    for index, row in enumerate(timeline):
        if (int(row["frame"]) != index or
                int(row["video_frame"]) != metadata["first_video_frame"] + index or
                abs(float(row["seconds"]) - float(index / rate)) > 1e-9 or
                int(row["input_dropped"]) != 0):
            parser.error(f"source timeline is not continuous at frame {index}")
    frames = sorted(directory.glob("frame-*.ppm"))
    expected_names = [f"frame-{index:06d}.ppm" for index in range(count)]
    if [frame.name for frame in frames] != expected_names:
        parser.error("source frames are missing or include another capture")
    for frame in frames:
        with frame.open("rb") as stream:
            if stream.read(15) != b"P6\n240 160\n255\n" or frame.stat().st_size != 115215:
                parser.error(f"{frame.name} is not a complete native RGB frame")
    rom = Path(metadata["rom"])
    elf = Path(metadata["elf"])
    if "elf_crc32" not in metadata:
        parser.error("capture is missing its ELF fingerprint; create a new capture")
    rom_bytes = rom.read_bytes()
    elf_bytes = elf.read_bytes()
    crc = f"{zlib.crc32(rom_bytes) & 0xffffffff:08x}"
    if crc != metadata["rom_crc32"]:
        parser.error("the source ROM changed after capture; cannot attribute this film")
    elf_crc = f"{zlib.crc32(elf_bytes) & 0xffffffff:08x}"
    if elf_crc != metadata["elf_crc32"]:
        parser.error("the source ELF changed after capture; cannot attribute this film")
    # Hash precisely the bytes that passed the capture-time checks. Reopening
    # either path after encoding could silently attribute a replaced file.
    rom_sha256 = hashlib.sha256(rom_bytes).hexdigest()
    elf_sha256 = hashlib.sha256(elf_bytes).hexdigest()
    del rom_bytes, elf_bytes
    stem = (args.output_stem or (directory / "opening")).resolve()
    stem.parent.mkdir(parents=True, exist_ok=True)
    mp4_path = Path(str(stem) + ".mp4")
    gif_path = Path(str(stem) + ".gif")
    manifest_path = Path(str(stem) + ".json")
    outputs = [mp4_path, manifest_path] + ([] if args.skip_gif else [gif_path])
    if any(path.exists() for path in outputs):
        parser.error("an output already exists; choose a fresh output stem")
    input_args = ["ffmpeg", "-hide_banner", "-loglevel", "warning", "-n",
                  "-framerate", f"{rate.numerator}/{rate.denominator}",
                  "-start_number", "0", "-i", str(directory / "frame-%06d.ppm")]
    scale = f"scale={240 * args.scale}:{160 * args.scale}:flags=neighbor"
    video_filter = scale if args.fps == "native" else f"fps={args.fps}:start_time=0,{scale}"
    mp4_command = input_args + [
        "-vf", video_filter, "-an", "-c:v", "libx264", "-crf", "15",
        "-pix_fmt", "yuv420p", "-movflags", "+faststart",
        "-video_track_timescale", str(metadata["cpu_frequency_hz"]), str(mp4_path),
    ]
    subprocess.run(mp4_command, check=True)
    mp4_rate = rate if args.fps == "native" else Fraction(int(args.fps))
    mp4_probe = verify_export(mp4_path, mp4_rate, duration,
                              240 * args.scale, 160 * args.scale,
                              count if args.fps == "native" else None)
    manifest = {
        "source_capture": str(meta_path),
        "source_capture_sha256": sha256(meta_path),
        "source_rom": str(rom),
        "source_rom_sha256": rom_sha256,
        "source_elf": str(elf),
        "source_elf_sha256": elf_sha256,
        "source_video_rate": str(rate),
        "source_frame_count": count,
        "source_duration_seconds": duration,
        "audio_included": False,
        "game_memory_writes": False,
        "display_scale": args.scale,
        "scaling_filter": "nearest-neighbor",
        "sampling_note": "20/30 FPS derivatives sample source timestamps; their final hold can round by less than one output interval",
        "mp4": {"path": str(mp4_path), "sha256": sha256(mp4_path),
                "requested_rate": args.fps, "probe": mp4_probe,
                "command": mp4_command},
    }
    if not args.skip_gif:
        gif_filter = (f"fps=20:start_time=0,{scale},split[a][b];"
                      "[a]palettegen=stats_mode=full:reserve_transparent=0[p];"
                      "[b][p]paletteuse=dither=none")
        gif_command = input_args + ["-filter_complex_threads", "1",
                                   "-filter_complex", gif_filter,
                                   "-an", "-loop", "0", str(gif_path)]
        subprocess.run(gif_command, check=True)
        gif_probe = verify_export(gif_path, Fraction(20), duration,
                                  240 * args.scale, 160 * args.scale)
        manifest["gif"] = {"path": str(gif_path), "sha256": sha256(gif_path),
                           "requested_rate": "20", "probe": gif_probe,
                           "command": gif_command}
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n")
    print(json.dumps({"mp4": str(mp4_path),
                      "gif": None if args.skip_gif else str(gif_path),
                      "manifest": str(manifest_path),
                      "source_rom_sha256": manifest["source_rom_sha256"],
                      "source_frames": count, "source_seconds": duration}, indent=2))


if __name__ == "__main__":
    main()
