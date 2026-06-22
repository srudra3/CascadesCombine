#!/usr/bin/env python3
"""
Make Regions YAML for 2L (grid) and 3L (rule-driven).

- Produces:
    Regions_2L_0J_hPTISR_Gold.yaml
    Regions_2L_1J_hPTISR_Gold.yaml
    Regions_3L_0J_hPTISR_Gold.yaml
    Regions_3L_1J_hPTISR_Gold.yaml

- 2L behavior: combinatorial generation in RISR and Mperp
- 3L behavior: generated from explicit rule table THREE_L_RULES

Run inside CMSSW (after cmsenv) where ruamel.yaml is available.
"""
from pathlib import Path
from ruamel.yaml import YAML
from ruamel.yaml.scalarstring import PreservedScalarString, DoubleQuotedScalarString
import argparse, subprocess

# ---------------- USER-TWEAKABLE CONSTANTS ----------------
OUTDIR = Path("config/bin_cfgs")

# ---------------- ERA CONFIG ----------------
ERAS = {
    "Run2": {
        "tag": "Run2",
        # future hooks:
        # "ptisr_2l": 350,
        # "ptisr_3l": 300,
        # "extra_tokens": [],
    },
    "Run3": {
        "tag": "Run3",
        # future hooks:
        # "ptisr_2l": 350,
        # "ptisr_3l": 300,
        # "extra_tokens": [],
    },
}

# PTISR tags used in names and cuts
PTISR_2L = 350
PTISR_3L = 300

# leave empty and apply in make_GSB
COMMON_BASE_TOKENS = []

PREDEFINED_CUTS = ""
USER_CUTS = ""

JET_CONFIGS = {
    "0J": {"njet_cond": "Njet_S==0;", "njet_note": "Njet_S==0;"},
    "1J": {"njet_cond": "Njet_S>=1;", "njet_note": "Njet_S>=1;"},
}

JET_CONFIGS_3L = {
    #"0J": {"njet_cond": "Njet_S==0;", "njet_note": "Njet_S==0;"},
    #"1J": {"njet_cond": "Njet_S>=1;", "njet_note": "Njet_S>=1;"},
    "Jincl": {"njet_cond": None}  # no jet cut
}

# ---------------- RISR / Mperp bins ----------------
RISR_BINS_2L = [
    {"name": "R95", "min": 0.95, "max": 1.0,  "Mh": 15, "Mm_lo": 5,  "Mm_hi": 15, "Ml_hi": 5},
    {"name": "R9",  "min": 0.9,  "max": 0.95, "Mh": 20, "Mm_lo": 10, "Mm_hi": 20, "Ml_hi": 10},
    {"name": "R85", "min": 0.85, "max": 0.9,  "Mh": 25, "Mm_lo": 15, "Mm_hi": 25, "Ml_hi": 15},
    {"name": "R8",  "min": 0.8,  "max": 0.85, "Mh": 30, "Mm_lo": 15, "Mm_hi": 30, "Ml_hi": 15},
    {"name": "R75", "min": 0.75, "max": 0.8,  "Mh": 30, "Mm_lo": 20, "Mm_hi": 30, "Ml_hi": 20},
    {"name": "R7",  "min": 0.7,  "max": 0.75, "Mh": 35, "Mm_lo": 25, "Mm_hi": 35, "Ml_hi": 25},
]

# 3L RISR/Mperp bins
RISR_BINS_3L = [
    {"name": "R9",  "min": 0.9,  "max": 1.0,  "Mh": 30, "Mm_lo": 30, "Mm_hi": 30, "Ml_hi": 30},
    {"name": "R8",  "min": 0.8,  "max": 0.9,  "Mh": 35, "Mm_lo": 35, "Mm_hi": 35, "Ml_hi": 35},
    {"name": "R7",  "min": 0.7,  "max": 0.8,  "Mh": 40, "Mm_lo": 40, "Mm_hi": 40, "Ml_hi": 40},
]

# 4L RISR/Mperp bins
RISR_BINS_4L = [
    {"name": "R7", "min": 0.7, "max": 1.0, "Mh": 40, "Ml_hi": 40},
]

NLEP_B_SPLITS_4L = [2, 1]  # order just for readability

# btag sideband
RISR_SB = [
    {"name": "R7", "min": 0.7, "max": 0.8},
    {"name": "R8", "min": 0.8, "max": 0.9},
    {"name": "R9", "min": 0.9, "max": 1.0},
]

PTISR_SB = {
    2: [(250, 350), (350, None)],
    3: [(200, 300), (300, None)],
    4: [(100, None)],
}

# 2L flavor splits
FLAVOR_SPLITS_2L = [
    ("OS_ee", ["=1OSSF;", "=2Elec;"]),
    ("OS_emu", ["=1OSOF;"]),
    ("OS_mumu", ["=1OSSF;", "=2Muon;"]),
    ("SS", ["AllSS;"]),
]

# 3L mapping from semantic labels to lep-cuts lines (used by rules)
LEP_CLASS_MAP_3L = {
    "incl": [],                # just the common lep block
    "OSOFa": ["=1OSOF_a;"],
    "nOSOFa": ["=0OSOF_a;"],
    "OSSFa": ["=1OSSF_a;"],
    "OSSFa_AllSF": ["=1OSSF_a;", "AllSF;"],
    "OSSFa_OSOF": ["=1OSSF_a;", "=1OSOF;"],
    "AllSF": ["AllSF;"],
    "AllSS": ["AllSS;"],
    "OFa": ["=1Elec_a;", "=1Muon_a;"],
    "OSa": ["=1Pos_a;", "=1Neg_a;"],
    "SSa": ["AllSS_a;"],
    "SFa": ["AllSF_a;"],
}

# default MaRatio parameters
DEFAULT_MARATIO_THRESHOLD = 0.3
MARATIO_NAME_A = "Ah"
MARATIO_NAME_L = "Al"

# ------------------ RULE TABLE for 3L ------------------
# Each entry is intentionally explicit to avoid cartesian explosions.
# Structure notes:
# - for each jet ('0J'/'1J') provide keys for RISR names (R9, R8, R7)
# - each RISR entry contains either:
#    - {'incl': True}  -> single inclusive bin (no Mperp)
#    - {'mperp': { 'Mh': {...}, 'Ml': {...}, ... } }
# - per mperp branch:
#    - 'use_mperp': bool (whether to add an Mperp token)
#    - 'mperp_cat': 'Mh'/'Ml' (used to lookup thresholds)
#    - 'maratio': bool (split Ah/Al)
#    - 'lep_classes': list of keys from LEP_CLASS_MAP_3L OR 'incl'
THREE_L_RULES = {
    "Jincl": {
        "R9": {
            "mperp": {
                "Mh": {
                    "use_mperp": True,
                    "mperp_cat": "Mh",
                    "maratio": False,
                    "lep_classes": ["OSOFa", "OSSFa", "SSa"],
                },
                "Ml": {
                    "use_mperp": True,
                    "mperp_cat": "Ml",
                    "maratio": False,
                    "lep_classes": ["OSOFa", "OSSFa", "SSa"],
                },
            }
        },
        "R8": {
            "mperp": {
                "Mh": {
                    "use_mperp": True,
                    "mperp_cat": "Mh",
                    "maratio": False,
                    "lep_classes": ["OSOFa", "OSSFa", "SSa"],
                },
                "Ml": {
                    "use_mperp": True,
                    "mperp_cat": "Ml",
                    "maratio": False,
                    "lep_classes": ["OSOFa", "OSSFa", "SSa"],
                },
            }
        },
        "R7": {
            "mperp": {
                "Mh": {
                    "use_mperp": True,
                    "mperp_cat": "Mh",
                    "maratio": False,
                    "lep_classes": ["OSOFa", "OSSFa", "SSa"],
                },
                "Ml": {
                    "use_mperp": True,
                    "mperp_cat": "Ml",
                    "maratio": False,
                    "lep_classes": ["OSOFa", "OSSFa", "SSa"],
                },
            }
        },
    },
}

# -------------------------------------------------------------------------

yaml = YAML()
yaml.default_flow_style = False
yaml.indent(mapping=2, sequence=4, offset=2)
yaml.width = 4096
yaml.preserve_quotes = True

# ---------------- helper utilities ----------------
def era_bin_name(era, base_name):
    """
    Insert era into a bin name.
    Example:
      Bin2L_Gold_0J_P250_R95_M15_OS_mumu
      -> Bin_Run2_2L_Gold_0J_P250_R95_M15_OS_mumu
    """
    if not base_name.startswith("Bin"):
        raise ValueError(f"Bin name does not start with 'Bin': {base_name}")
    return f"Bin_{era}_{base_name[3:]}"

def format_rISR_condition(r):
    if r["max"] >= 1.0:
        return f"RISR_LEP>={r['min']};RISR_LEP<=1.;"
    else:
        return f"RISR_LEP>={r['min']};RISR_LEP<{r['max']};"

def make_mperp_cut_tokens(r, category):
    if category == "Mh":
        cut = f"Mperp_LEP>={r['Mh']}"
        name_frag = f"M{r['Mh']}"
    elif category == "Mm":
        cut = f"Mperp_LEP>={r['Mm_lo']};Mperp_LEP<{r['Mm_hi']}"
        name_frag = f"M{r['Mm_lo']}_{r['Mm_hi']}"
    elif category == "Ml":
        cut = f"Mperp_LEP<{r['Ml_hi']}"
        name_frag = f"Mlt{r['Ml_hi']}"
    else:
        raise ValueError("unknown category")
    if not cut.endswith(";"):
        cut += ";"
    return cut, name_frag

def assemble_cuts_string(nlep, base_tokens, ptisr_token, risr_token, jet_token, mperp_token, extra_tokens=None):
    parts = []
    parts.append(f"Nlep=={nlep};")
    parts.extend(base_tokens)
    parts.append(ptisr_token)
    parts.append(risr_token)
    parts.append("METORtrigger==1;")
    parts.append("EventFilter==1;")
    parts.append("PassesJVM==1;")
    if jet_token:
        parts.append(jet_token)
    parts.append("MIN(abs(Eta_lep))<1.4442;")
    parts.append("Njet>0;")
    if mperp_token:
        parts.append(mperp_token)
    if extra_tokens:
        parts.extend(extra_tokens)
    # preserve order, dedupe
    seen = set()
    out = []
    for p in parts:
        p = p.strip()
        if p == "":
            continue
        if not p.endswith(";"):
            p = p + ";"
        if p not in seen:
            seen.add(p)
            out.append(p)
    return "".join(out)

def make_lep_cuts_block(common_lep_lines, flavor_extra):
    lines = []
    for l in common_lep_lines:
        if not l.endswith(";"):
            l = l + ";"
        lines.append(l)
    for fe in flavor_extra:
        if not fe.endswith(";"):
            fe = fe + ";"
        lines.append(fe)
    return PreservedScalarString("\n".join(lines) + "\n")

# find risr dict by name in a given list
def lookup_risr_by_name(name, risr_list):
    for r in risr_list:
        if r["name"] == name:
            return r
    return None

# ---------------- 2L combinatorial generator ----------------
def build_bins_2l_for_jet(era, jtag, jconf, ptisr_tag, risr_bins):
    out = {}
    PTISR_CUT_STR = f"PTISR_LEP>={ptisr_tag};"
    PTISR_PNAME = f"P{ptisr_tag}"
    COMMON_LEP = ["=0OSSF|mass>=3|mass<=3.2;", "=2Gold;"]
    for r in risr_bins:
        risr_cond = format_rISR_condition(r)
        for mcat in ("Mh", "Mm", "Ml"):
            if mcat == "Mm" and r["Mm_lo"] >= r["Mm_hi"]: # skip middle bin if its bin boundaries are the hi/lo
                continue
            mcut_token, mnamefrag = make_mperp_cut_tokens(r, mcat)
            for (flav_key, flavor_lep_lines) in FLAVOR_SPLITS_2L:
                raw_key = f"Bin2L_Gold_{jtag}_{PTISR_PNAME}_{r['name']}_{mnamefrag}_{flav_key}"
                bin_key = era_bin_name(era, raw_key)
                jet_token = jconf["njet_cond"]
                cuts_txt = DoubleQuotedScalarString(
                    assemble_cuts_string(
                        2,
                        COMMON_BASE_TOKENS,
                        PTISR_CUT_STR,
                        risr_cond,
                        jet_token,
                        mcut_token,
                        extra_tokens=["Nbjet==0;"],
                    )
                )
                lep_block = make_lep_cuts_block(COMMON_LEP, flavor_lep_lines)
                out[bin_key] = {
                    "cuts": cuts_txt,
                    "lep-cuts": lep_block,
                    "predefined-cuts": DoubleQuotedScalarString(PREDEFINED_CUTS),
                    "user-cuts": DoubleQuotedScalarString(USER_CUTS),
                }
    return out

# ---------------- 3L rule-driven generator ----------------
def build_bins_3l_from_rules(era, jtag, jconf, ptisr_tag, risr_bins, rules_for_jet, maratio_threshold, verbose=False):
    """
    rules_for_jet: dict for that jet from THREE_L_RULES
    risr_bins: list (RISR_BINS_3L)
    """
    out = {}
    COMMON_LEP = ["=0OSSF|mass>=3|mass<=3.2;", "=3Gold;"]
    PTISR_CUT_STR = f"PTISR_LEP>={ptisr_tag};"
    PTISR_PNAME = f"P{ptisr_tag}"

    # iterate over rules for this jet
    for risr_name, risr_rule in rules_for_jet.items():
        r = lookup_risr_by_name(risr_name, risr_bins)
        if r is None:
            # safety: skip unknown risr names
            if verbose:
                print(f"[WARN] No RISR definition found for {risr_name} in risr_bins; skipping")
            continue
        risr_cond = format_rISR_condition(r)

        # inclusive-only branch (no Mperp token)
        if risr_rule.get("incl", False):
            bin_key = f"Bin3L_Gold_{jtag}_{PTISR_PNAME}_{r['name']}"
            cuts_txt = DoubleQuotedScalarString(
                assemble_cuts_string(
                    3,
                    COMMON_BASE_TOKENS,
                    PTISR_CUT_STR,
                    risr_cond,
                    jconf["njet_cond"],
                    mperp_token=None,
                    extra_tokens=["Nbjet==0;"],
                )
            )
            lep_block = make_lep_cuts_block(COMMON_LEP, [])  # no extra flavor lines
            out[bin_key] = {
                "cuts": cuts_txt,
                "lep-cuts": lep_block,
                "predefined-cuts": DoubleQuotedScalarString(PREDEFINED_CUTS),
                "user-cuts": DoubleQuotedScalarString(USER_CUTS),
            }
            continue

        # otherwise expect a 'mperp' subdict
        mperp_dict = risr_rule.get("mperp", {})
        for mlabel, mrule in mperp_dict.items():
            use_mperp = mrule.get("use_mperp", True)
            mperp_cat = mrule.get("mperp_cat", mlabel)  # usually 'Mh' or 'Ml'
            mcut_token = None
            mnamefrag = ""
            if use_mperp:
                # ---------------------- Fixed MaRatio / 3L block ----------------------
                mcut_token, mnamefrag = (None, "")
                if use_mperp:
                    mcut_token, mnamefrag = make_mperp_cut_tokens(r, mperp_cat)
                
                maratio_cfg = mrule.get("maratio", False)
                
                # default: no MaRatio, just use lep_classes
                if not maratio_cfg:
                    maratio_branches = [("", None, mrule.get("lep_classes", ["incl"]))]
                else:
                    if isinstance(maratio_cfg, dict):
                        # dict-based MaRatio: Ah/Al branches
                        maratio_branches = [
                            ("_Ah", f"MaRatio_LEP>={maratio_threshold};", maratio_cfg.get("Ah", [])),
                            ("_Al", f"MaRatio_LEP<{maratio_threshold};", maratio_cfg.get("Al", [])),
                        ]
                    else:
                        # legacy boolean MaRatio
                        maratio_branches = [
                            ("_Ah", f"MaRatio_LEP>={maratio_threshold};", mrule.get("lep_classes", ["incl"])),
                            ("_Al", f"MaRatio_LEP<{maratio_threshold};", mrule.get("lep_classes", ["incl"])),
                        ]
                
                # now loop over branches
                for maratio_suffix, maratio_token, lep_classes in maratio_branches:
                    for lc in lep_classes:
                        flavor_lines = []
                        if lc != "incl":
                            flavor_lines = LEP_CLASS_MAP_3L.get(lc, [])
                
                        # build name: include mnamefrag from mperp
                        if use_mperp:
                            if lc == "incl" and maratio_suffix == "":
                                name_part = f"{mnamefrag}"
                            else:
                                name_part = f"{mnamefrag}{maratio_suffix}_{lc}"
                        else:
                            if lc == "incl" and maratio_suffix == "":
                                name_part = ""
                            else:
                                name_part = f"{maratio_suffix}_{lc}".lstrip("_")
                
                        # create bin key
                        if name_part:
                            bin_key = f"Bin3L_Gold_{jtag}_{PTISR_PNAME}_{r['name']}_{name_part}"
                        else:
                            bin_key = f"Bin3L_Gold_{jtag}_{PTISR_PNAME}_{r['name']}"
                
                        # assemble cuts
                        extra_tokens = []
                        if maratio_token:
                            extra_tokens.append(maratio_token)
                        extra_tokens.append("Nbjet==0;"),
                        cuts_txt = DoubleQuotedScalarString(
                            assemble_cuts_string(
                                3,
                                COMMON_BASE_TOKENS,
                                PTISR_CUT_STR,
                                risr_cond,
                                jconf["njet_cond"],
                                mcut_token if use_mperp else None,
                                extra_tokens=extra_tokens if extra_tokens else None,
                            )
                        )
                        lep_block = make_lep_cuts_block(COMMON_LEP, flavor_lines)
                        out[era_bin_name(era, bin_key)] = {
                            "cuts": cuts_txt,
                            "lep-cuts": lep_block,
                            "predefined-cuts": DoubleQuotedScalarString(PREDEFINED_CUTS),
                            "user-cuts": DoubleQuotedScalarString(USER_CUTS),
                        }
    return out

# ---------------- 4L generator ----------------
def build_bins_4l(era, ptisr_tag=100):
    out = {}
    COMMON_LEP = ["=0OSSF|mass>=3|mass<=3.2;", "=4Gold;"]

    PTISR_CUT = f"PTISR_LEP>={ptisr_tag};"
    MET_CUT = "MET>=150;"
    JET_CUT = "Njet_S==0;"
    EXTRA_BASE = ["Njet>0;"]  # matches your YAML

    for r in RISR_BINS_4L:
        risr_cond = format_rISR_condition(r)

        for mcat in ("Ml", "Mh"):
            mcut, mname = make_mperp_cut_tokens(r, mcat)

            for nb in NLEP_B_SPLITS_4L:
                raw_key = f"Bin4L_Gold_{r['name']}_{mname}"
                if   nb == 2: raw_key += "_22"
                elif nb == 1: raw_key += "_31"
                key = era_bin_name(era, raw_key)
                cuts = DoubleQuotedScalarString(
                    assemble_cuts_string(
                        4,
                        COMMON_BASE_TOKENS + [MET_CUT],
                        PTISR_CUT,
                        risr_cond,
                        JET_CUT,
                        mcut,
                        extra_tokens=[f"Nlep_b_LEP=={nb};Nbjet==0;"] + EXTRA_BASE,
                    )
                )

                out[key] = {
                    "cuts": cuts,
                    "lep-cuts": make_lep_cuts_block(COMMON_LEP, []),
                    "predefined-cuts": DoubleQuotedScalarString("Cleaning_LEP;dphiMETV_LEP;"),
                    "user-cuts": DoubleQuotedScalarString("minMll_minDR_2D_low;HEM_Veto;leadSjet_pt;"),
                }
    return out

# ---------------- sideband generator ----------------
def build_bins_btag_sideband(era):
    out = {}

    for nlep in (2, 3, 4):
        COMMON_LEP = ["=0OSSF|mass>=3|mass<=3.2;", f"={nlep}Gold;"]

        for (ptlo, pthi) in PTISR_SB[nlep]:
            if pthi:
                ptcut = f"PTISR_LEP>={ptlo};PTISR_LEP<{pthi};"
                pname = f"P{ptlo}"
            else:
                ptcut = f"PTISR_LEP>={ptlo};"
                pname = f"P{ptlo}"

            # -------------------------------------------------
            # 4L: inclusive jets + single RISR bin 0.7->1.0
            # -------------------------------------------------
            if nlep == 4:
                risr_cond = "RISR_LEP>=0.7;RISR_LEP<=1.;"
                raw_key = f"Bin4L_Gold_{pname}_R7_Btag"
                key = era_bin_name(era, raw_key)

                cuts = DoubleQuotedScalarString(
                    assemble_cuts_string(
                        4,
                        COMMON_BASE_TOKENS + ["MET>=150;", "Njet>0;"],
                        ptcut,
                        risr_cond,
                        "Njet_S==0;",
                        None,
                        extra_tokens=["Nbjet>=1;"],
                    )
                )

                out[key] = {
                    "cuts": cuts,
                    "lep-cuts": make_lep_cuts_block(COMMON_LEP, []),
                    "predefined-cuts": DoubleQuotedScalarString("Cleaning_LEP;dphiMETV_LEP;"),
                    "user-cuts": DoubleQuotedScalarString("minMll_minDR_2D_low;HEM_Veto;leadSjet_pt;"),
                }

                continue

            # -------------------------------------------------
            # 3L: inclusive jets, keep RISR_SB bins
            # -------------------------------------------------
            if nlep == 3:
                jet_iter = [("Jincl", {"njet_cond": None})]
                risr_iter = RISR_SB
            else:
                # 2L unchanged
                jet_iter = JET_CONFIGS.items()
                risr_iter = RISR_SB

            for r in risr_iter:
                risr_cond = format_rISR_condition(r)

                for jtag, jconf in jet_iter:
                    raw_key = f"Bin{nlep}L_Gold_{jtag}_{pname}_{r['name']}_Btag"
                    key = era_bin_name(era, raw_key)

                    cuts = DoubleQuotedScalarString(
                        assemble_cuts_string(
                            nlep,
                            COMMON_BASE_TOKENS + ["MET>=150;", "Njet>0;"],
                            ptcut,
                            risr_cond,
                            jconf["njet_cond"],
                            None,
                            extra_tokens=["Nbjet>=1;"],
                        )
                    )

                    out[key] = {
                        "cuts": cuts,
                        "lep-cuts": make_lep_cuts_block(COMMON_LEP, []),
                        "predefined-cuts": DoubleQuotedScalarString("Cleaning_LEP;dphiMETV_LEP;"),
                        "user-cuts": DoubleQuotedScalarString("minMll_minDR_2D_low;HEM_Veto;leadSjet_pt;"),
                    }
    return out

# ---------------- file writing helper ----------------
def write_yaml_map(filename: Path, mapping):
    filename.parent.mkdir(parents=True, exist_ok=True)
    with filename.open("w", encoding="utf-8") as f:
        yaml.dump(mapping, f)
    print("WROTE:", filename)

# ---------------- main entrypoint ----------------
def main(outdir: Path, dry_run=False, maratio_threshold=DEFAULT_MARATIO_THRESHOLD, verbose=False):
    files_to_write = {}

    for era, era_cfg in ERAS.items():
        # 2L
        for jtag, jconf in JET_CONFIGS.items():
            doc_map = build_bins_2l_for_jet(
                era, jtag, jconf, PTISR_2L, RISR_BINS_2L
            )
            fname = outdir / f"Regions_{era}_2L_{jtag}_hPTISR_Gold.yaml"
            files_to_write[fname] = doc_map
        # 3L
        for jtag, jconf in JET_CONFIGS_3L.items():
            rules_for_jet = THREE_L_RULES.get(jtag, {})
            doc_map = build_bins_3l_from_rules(
                era,
                jtag,
                jconf,
                PTISR_3L,
                RISR_BINS_3L,
                rules_for_jet,
                maratio_threshold,
                verbose=verbose,
            )
            fname = outdir / f"Regions_{era}_3L_{jtag}_hPTISR_Gold.yaml"
            files_to_write[fname] = doc_map
        # 4L
        files_to_write[outdir / f"Regions_{era}_4L_Gold.yaml"] = build_bins_4l(era)
        # b-tag sideband
        files_to_write[outdir / f"Regions_{era}_top_sideband_Gold.yaml"] = build_bins_btag_sideband(era)

    # write or dry-run
    for path, doc in files_to_write.items():
        if dry_run:
            print(f"[DRY-RUN] would write {path} with {len(doc)} bins")
            # also print sample keys for quick sanity
            sample_keys = list(doc.keys())[:12]
            if sample_keys:
                print("  sample keys:", sample_keys)
        else:
            write_yaml_map(path, doc)

    # call generator for corresponding Gold, Silver, and Bronze
    try:
        make_GSB_cmd = ["python3", "python/make_GSB_Regions.py"]
        print("making other regions: python3 python/make_GSB_Regions.py")
        subprocess.run(make_GSB_cmd, check=True, text=True)
    except Exception as e:
        print("skipping make_GSB_Regions.py (error calling it):", e)
    # call generator for consolidations
    try:
        print("making region consolidations")
        make_Consolidated_cmd_0 = ["python3", "python/make_Consolidated_Regions.py", "config/bin_cfgs/Regions_Run2_2L_0J_lPTISR_Silver.yaml", "config/bin_cfgs/Regions_Run2_2L_1J_hPTISR_Silver.yaml"]
        subprocess.run(make_Consolidated_cmd_0, check=True, text=True)
        make_Consolidated_cmd_1 = ["python3", "python/make_Consolidated_Regions.py", "config/bin_cfgs/Regions_Run3_2L_0J_lPTISR_Silver.yaml", "config/bin_cfgs/Regions_Run3_2L_1J_hPTISR_Silver.yaml"]
        subprocess.run(make_Consolidated_cmd_1, check=True, text=True)
        make_Consolidated_cmd_2 = ["python3", "python/make_Consolidated_Regions.py", "config/bin_cfgs/Regions_Run2_3L_Jincl_lPTISR_Silver.yaml", "config/bin_cfgs/Regions_Run2_3L_Jincl_hPTISR_Silver.yaml"]
        subprocess.run(make_Consolidated_cmd_2, check=True, text=True)
        make_Consolidated_cmd_3 = ["python3", "python/make_Consolidated_Regions.py", "config/bin_cfgs/Regions_Run3_3L_Jincl_lPTISR_Silver.yaml", "config/bin_cfgs/Regions_Run3_3L_Jincl_hPTISR_Silver.yaml"]
        subprocess.run(make_Consolidated_cmd_3, check=True, text=True)
    except Exception as e:
        print("skipping make_Consolidated_Regions.py (error calling it):", e)
    print("DONE MAKING REGIONS YAMLS")

if __name__ == "__main__":
    ap = argparse.ArgumentParser(description="Make Regions (2L grid + 3L rule-driven)")
    ap.add_argument("--outdir", "-o", default=str(OUTDIR))
    ap.add_argument("--dry-run", action="store_true", help="Don't write files, just report counts")
    ap.add_argument("--maratio-threshold", type=float, default=DEFAULT_MARATIO_THRESHOLD,
                    help="MaRatio threshold used where MaRatio is enabled in rules (default %(default)s)")
    ap.add_argument("--verbose", action="store_true", help="Verbose logging for debugging rule application")
    args = ap.parse_args()

    main(Path(args.outdir), dry_run=args.dry_run, maratio_threshold=args.maratio_threshold, verbose=args.verbose)

