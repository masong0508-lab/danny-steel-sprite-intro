#!/usr/bin/env python3
"""Regress the real source's cropped-joint and palm-contact cases."""
import copy
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("trace_pack", ROOT / "tools/build_potheads_trace.py")
PACK = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PACK)


def fixture():
    pose = {
        "frame_index": 0, "source_timestamp_s": 0.05, "tick_60": 0,
        "points": [[300, 320, 2] for _ in PACK.NAMES],
        "facing": "front", "near_arm": "right", "grounded": True,
        "contacts": {"left": None, "right": None, "left_hand": [210, 358, 2]},
        "sign": {"state": "hidden", "layer": "front_torso", "quad": None},
    }
    pose["points"][10] = [None, None, 0]
    pose["clipped_boundary"] = {"left_ankle": [640, 360, 1]}
    end = copy.deepcopy(pose)
    end.update(frame_index=547, source_timestamp_s=18.27, tick_60=1093,
               clipped_boundary={})
    return {"status": "complete", "source": {"frame_count": 548, "sha256": "fixture"},
            "landmark_order": PACK.NAMES, "poses": [pose, end]}


class CropPackingTest(unittest.TestCase):
    def test_cropped_ankle_never_becomes_a_visible_origin_or_grounded_foot(self):
        data = fixture()
        header = PACK.build(data, json.dumps(data).encode())
        with tempfile.TemporaryDirectory() as folder:
            folder = Path(folder)
            (folder / "motion.h").write_text(header)
            (folder / "check.c").write_text('''#include "motion.h"
#include <assert.h>
int main(void){
 PotPose p;
 pot_trace_sample(0,&p);
 assert(p.contact_mask==POT_LEFT_PALM);
 assert((p.boundary&(1u<<POT_LANKLE))!=0u);
 assert((p.visible&(1u<<POT_LANKLE))==0u);
 assert((p.inferred&(1u<<POT_LANKLE))==0u);
 pot_trace_sample(1000,&p);
 assert(p.joint[POT_LANKLE].x==113); /* Hold observed crop, never drag to zero. */
 pot_trace_sample(1093,&p);
 assert((p.located&(1u<<POT_LANKLE))==0u);
 pot_trace_sample(65535,&p);
 assert((p.located&(1u<<POT_LANKLE))==0u); /* No loop. */
 assert(p.contact_mask==POT_LEFT_PALM);
 return 0;
}
''')
            subprocess.run(["gcc", "-std=c11", "-Wall", "-Wextra", "-Werror",
                            str(folder / "check.c"), "-o", str(folder / "check")], check=True)
            subprocess.run([str(folder / "check")], check=True)

    def test_full_reference_contains_direct_intermediate_standing_poses(self):
        data = json.loads((ROOT / "tools/references/potheads_trace.json").read_bytes())
        self.assertEqual(data["source"]["frame_count"], 548)
        self.assertEqual(data["coverage"]["pose_count"], len(data["poses"]))
        poses = {p["frame_index"]: p for p in data["poses"]}
        self.assertTrue(all(frame in poses for frame in range(0, 101, 5)))
        self.assertEqual(len(poses), 120)
        for frame in range(5, 100, 10):
            p = poses[frame]
            self.assertEqual(p["tick_60"], frame * 2)
            self.assertEqual(p["sign"]["state"], "held")
            self.assertEqual(len(p["sign"]["quad"]), 4)
            self.assertIn("exact decoded reference frame", p["note"])
        # Actual sign swing maximum and right-foot lift between old samples.
        self.assertGreater(poses[15]["sign"]["arrow_tip"][0], 500)
        self.assertIsNone(poses[95]["contacts"]["right"])
        self.assertEqual(poses[95]["points"][4][2], 1)
        self.assertEqual(poses[547]["tick_60"], 1093)

    def test_release_rejects_the_short_hop_only_excerpt(self):
        data = fixture()
        data["poses"][-1]["frame_index"] = 280
        with self.assertRaisesRegex(ValueError, "full source clip"):
            PACK.build(data, b"fixture")


if __name__ == "__main__":
    unittest.main()
