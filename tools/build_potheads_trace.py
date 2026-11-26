#!/usr/bin/env python3
"""Pack the full measured dance into a small, allocation-free ROM track.

JSON retains frame provenance and uncertainty. A cropped contour intersection
is separate from an unavailable joint. No raster/video is embedded. Review the
native film after generation: valid coordinates alone do not establish acting.
"""
import argparse
import hashlib
import json
from pathlib import Path

NAMES = (
    "head", "neck", "left_shoulder", "left_elbow", "left_wrist",
    "right_shoulder", "right_elbow", "right_wrist", "left_hip",
    "left_knee", "left_ankle", "right_hip", "right_knee", "right_ankle",
    "left_toe", "right_toe",
)
CONTACTS = ("left", "right", "left_hand", "right_hand")


def known(point):
    return point is not None and point[0] is not None and point[1] is not None


def quantize(point):
    values = [round((point[0] - 300) / 3), round((point[1] - 350) / 3)]
    if not all(-128 <= value <= 127 for value in values):
        raise ValueError(f"Located point outside signed-byte range: {point}")
    return values


def coords(values):
    return "{" + ",".join("{%d,%d}" % tuple(point) for point in values) + "}"


def pair(value):
    return "{%d,%d}" % tuple(value)


def pack_pose(pose, last_sign):
    points, visible, inferred, boundary = [], 0, 0, 0
    boundaries = pose.get("clipped_boundary", {})
    for i, point in enumerate(pose["points"]):
        if known(point):
            if point[2] not in (1, 2):
                raise ValueError("A located anatomical joint needs visibility 1 or 2")
            points.append(quantize(point))
            if point[2] == 2:
                visible |= 1 << i
            else:
                inferred |= 1 << i
        elif known(boundaries.get(NAMES[i])):
            points.append(quantize(boundaries[NAMES[i]]))
            boundary |= 1 << i
        else:
            points.append([0, 0])  # Masked; never paint or interpolate it.
    contact, contact_mask = [], 0
    for i, name in enumerate(CONTACTS):
        point = pose.get("contacts", {}).get(name)
        if known(point):
            contact.append(quantize(point))
            contact_mask |= 1 << i
        else:
            contact.append([0, 0])
    sign = pose["sign"]
    held = sign["state"] in ("held", "holding")
    falling = sign["state"] in ("falling", "released", "free", "detached")
    quad = sign.get("quad")
    if quad and all(known(point) for point in quad):
        tip = sign.get("arrow_tip")
        if not known(tip):
            tip = [quad[1][0], (quad[1][1] + quad[2][1]) / 2, 1]
        last_sign = [quantize(point) for point in [*quad, tip]]
    facing = {"front": 0, "left": 1, "right": 2, "back": 3}[pose["facing"]]
    flags = ((1 if held else 2 if falling else 0)
             | (4 if sign["layer"] in ("front", "front_torso", "in_front") else 0)
             | (8 if pose.get("near_arm") == "left" else 0)
             | (facing << 4) | (64 if pose.get("grounded") else 0)
             | (128 if pose.get("near_leg", "left") == "left" else 0))
    bag = pose.get("bag") or {}
    centre = bag.get("centre", bag.get("center"))
    bag_state = {"hidden": 0, "attached": 1, "falling": 2, "resting": 3}.get(
        bag.get("state", "attached"), 0)
    if not known(centre):
        # Authored attachment before separately annotated bag samples.
        hip = pose["points"][8]
        centre = [hip[0] + 12, hip[1] - 20, 1] if known(hip) else None
    bag_point = quantize(centre) if known(centre) else [0, 0]
    if not known(centre):
        bag_state = 0
    bag_size = [6, 8]
    envelope = bag.get("quad", bag.get("envelope"))
    if envelope and all(known(point) for point in envelope):
        xs, ys = zip(*[point[:2] for point in envelope])
        bag_size = [max(2, min(20, round((max(xs) - min(xs)) / 6))),
                    max(2, min(20, round((max(ys) - min(ys)) / 6)))]
    head = pose.get("head_size") or [19, 23]
    head_size = [max(2, min(16, round(value / 3))) for value in head[:2]]
    row = (" {%du,%du,0x%04xu,0x%04xu,0x%04xu,%du,%du,%s,%s,%s,%s,%s,%s},"
           " /* frame %d; %.4fs */") % (
        pose["tick_60"], flags, visible, inferred, boundary, contact_mask,
        bag_state, coords(points), coords(contact), coords(last_sign),
        pair(bag_point), pair(bag_size), pair(head_size),
        pose["frame_index"], pose["source_timestamp_s"])
    return row, last_sign


def build(data, raw, preview=False):
    complete = data.get("status") == "complete"
    if not complete and not preview:
        raise ValueError("Only a completed, reviewed trace may become release data")
    if tuple(data["landmark_order"]) != NAMES:
        raise ValueError("Unexpected landmark order")
    poses = data["poses"]
    if not poses or any(b["tick_60"] <= a["tick_60"] for a, b in zip(poses, poses[1:])):
        raise ValueError("Source timestamps must increase strictly")
    if complete and (poses[0]["frame_index"] != 0 or
                     poses[-1]["frame_index"] != data["source"]["frame_count"] - 1):
        raise ValueError("Release data must cover the full source clip")
    rows, last_sign = [], [[0, 0] for _ in range(5)]
    for pose in poses:
        if len(pose["points"]) != len(NAMES):
            raise ValueError("Every pose needs all 16 joint visibility entries")
        row, last_sign = pack_pose(pose, last_sign)
        rows.append(row)
    header = """#pragma once
#include <stdint.h>

/* Measured reference: 307530.mp4. Full frame provenance, confidence and
   crop boundaries live in tools/references/potheads_trace.json.
   Generated by tools/build_potheads_trace.py. One unit = 3 reference pixels,
   relative to (300,350) in a 640x360 view. No raster/video is embedded. */
enum {
 POT_HEAD,POT_NECK,POT_LSHOULDER,POT_LELBOW,POT_LWRIST,
 POT_RSHOULDER,POT_RELBOW,POT_RWRIST,POT_LHIP,POT_LKNEE,
 POT_LANKLE,POT_RHIP,POT_RKNEE,POT_RANKLE,POT_LTOE,POT_RTOE,
 POT_JOINTS
};
enum { POT_SIGN_HELD=1,POT_SIGN_FALLING=2,POT_SIGN_FRONT=4,
       POT_NEAR_LEFT=8,POT_FACING_SHIFT=4,POT_GROUNDED=64,
       POT_NEAR_LEFT_LEG=128 };
enum { POT_BAG_HIDDEN,POT_BAG_ATTACHED,POT_BAG_FALLING,POT_BAG_RESTING };
enum { POT_LEFT_SOLE=1,POT_RIGHT_SOLE=2,POT_LEFT_PALM=4,POT_RIGHT_PALM=8 };
typedef struct {
 uint16_t tick,flags,visible,inferred,boundary;
 uint8_t contact_mask,bag_state;
 int8_t joint[POT_JOINTS][2],contact[4][2],sign[5][2],bag[2];
 uint8_t bag_size[2],head_size[2];
} PotTraceFrame;
"""
    header += f"#define POT_TRACE_COMPLETE {int(complete)}\n"
    header += f"#define POT_TRACE_COUNT {len(rows)}u\n"
    header += f"#define POT_TRACE_LAST_TICK {poses[-1]['tick_60']}u\n"
    header += f"#define POT_TRACE_LAST_SOURCE_FRAME {poses[-1]['frame_index']}u\n"
    header += f"/* Source video SHA256: {data['source']['sha256']} */\n"
    header += f"/* Measured JSON SHA256: {hashlib.sha256(raw).hexdigest()} */\n"
    header += "static const PotTraceFrame pot_trace[POT_TRACE_COUNT]={\n"
    header += "\n".join(rows) + "\n};\n"
    header += """
typedef struct { int x,y; } PotJoint;
typedef struct {
 unsigned flags,located,visible,inferred,boundary,contact_mask,bag_state;
 PotJoint joint[POT_JOINTS],contact[4],sign[5],bag,bag_size,head_size;
} PotPose;
static inline int pot_trace_lerp(int a,int b,unsigned n,unsigned d){
 /* Round the absolute interpolated coordinate, not the signed travel
    delta. Reversing a limb's direction must not move its half-tick pixel. */
 if(!d||!n||a==b)return a;
 if(n==d)return b;
 int numerator=a*(int)d+(b-a)*(int)n;
 /* For the measured short intervals the exact rounded quotient uses a
    bounded unsigned reciprocal. ARM7 has no integer divide instruction.
    ceil(65536/d) gives the exact floor for magnitude <=4095, d<=16:
    its error is strictly below 1/16, the smallest possible fraction.
    Retain the original division for all larger/custom intervals. */
 if(d<=16u){
  static const uint32_t inverse[17]={
   0u,65536u,32768u,21846u,16384u,13108u,10923u,9363u,8192u,
   7282u,6554u,5958u,5462u,5042u,4682u,4370u,4096u
  };
  unsigned magnitude=(numerator<0?0u-(unsigned)numerator:(unsigned)numerator)+d/2u;
  if(magnitude<=4095u){
   int q=(int)((magnitude*inverse[d])>>16);
   return numerator<0?-q:q;
  }
 }
 return (numerator+(numerator<0?-(int)(d/2u):(int)(d/2u)))/(int)d;
}
static inline void pot_trace_sample(unsigned tick,PotPose *out){
 unsigned lo=0u,hi=POT_TRACE_COUNT-1u;
 if(tick>pot_trace[hi].tick)tick=pot_trace[hi].tick;
 while(lo+1u<hi){
  unsigned mid=(lo+hi)/2u;
  if(pot_trace[mid].tick<=tick)lo=mid;else hi=mid;
 }
 const PotTraceFrame *a=&pot_trace[lo],*b=&pot_trace[hi];
 if(tick>=b->tick)a=b;
 unsigned n=tick-a->tick,d=b->tick-a->tick;
 unsigned located_b=b->visible|b->inferred|b->boundary;
 out->flags=a->flags;
 out->visible=a->visible;out->inferred=a->inferred;
 out->boundary=a->boundary;
 out->located=a->visible|a->inferred|a->boundary;
 out->contact_mask=a->contact_mask;out->bag_state=a->bag_state;
 for(unsigned i=0;i<POT_JOINTS;i++){
  /* Missing joints never pull a limb to zero. Boundary intersections
     continue cropped limbs without claiming unseen anatomical positions. */
  int both=(out->located&located_b&(1u<<i))!=0u;
  out->joint[i].x=both?pot_trace_lerp(a->joint[i][0],b->joint[i][0],n,d):a->joint[i][0];
  out->joint[i].y=both?pot_trace_lerp(a->joint[i][1],b->joint[i][1],n,d):a->joint[i][1];
 }
 for(unsigned i=0;i<4u;i++){
  int both=(a->contact_mask&b->contact_mask&(1u<<i))!=0u;
  out->contact[i].x=both?pot_trace_lerp(a->contact[i][0],b->contact[i][0],n,d):a->contact[i][0];
  out->contact[i].y=both?pot_trace_lerp(a->contact[i][1],b->contact[i][1],n,d):a->contact[i][1];
 }
 for(unsigned i=0;i<5u;i++){
  int keep=(a->flags&3u)&&!(b->flags&3u);
  out->sign[i].x=keep?a->sign[i][0]:pot_trace_lerp(a->sign[i][0],b->sign[i][0],n,d);
  out->sign[i].y=keep?a->sign[i][1]:pot_trace_lerp(a->sign[i][1],b->sign[i][1],n,d);
 }
 out->bag.x=pot_trace_lerp(a->bag[0],b->bag[0],n,d);
 out->bag.y=pot_trace_lerp(a->bag[1],b->bag[1],n,d);
 out->bag_size.x=pot_trace_lerp(a->bag_size[0],b->bag_size[0],n,d);
 out->bag_size.y=pot_trace_lerp(a->bag_size[1],b->bag_size[1],n,d);
 out->head_size.x=pot_trace_lerp(a->head_size[0],b->head_size[0],n,d);
 out->head_size.y=pot_trace_lerp(a->head_size[1],b->head_size[1],n,d);
}
"""
    return header


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--preview", action="store_true",
                        help="allow unfinished annotation for internal preview only")
    args = parser.parse_args()
    raw = args.source.read_bytes()
    data = json.loads(raw)
    args.output.write_text(build(data, raw, args.preview))
    print(f"{len(data['poses'])} measured poses; "
          f"{data['poses'][-1]['tick_60']} source-video ticks; "
          "coordinate data only (no raster or video)")


if __name__ == "__main__":
    main()
