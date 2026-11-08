import uproot
import matplotlib.pyplot as plt
import numpy as np

# Open ROOT file
print("Opening ROOT file...")
root_file = uproot.open('runs/ZZCR_10Sep/final_hadded.root')

# Define histograms to plot (with ;1 suffix)
samples = {
    "ZZ_2022": "M2lZVeto_ZZCR__ZZ_2022__quad_m4l;1",
    "DY_2022": "M2lZVeto_ZZCR__DY_2022__quad_m4l;1",
    "ttbar_2022": "M2lZVeto_ZZCR__ttbar_2022__quad_m4l;1",
}

# Define colors
colors = {
    "ZZ_2022": "#DC143C",
    "DY_2022": "#FFA500",
    "ttbar_2022": "#228B22",
}

fig, ax = plt.subplots(figsize=(13, 8))

for sample, hist_key in samples.items():
    try:
        hist = root_file[hist_key]
        print(f"✓ Plotting {sample}...")
        values = hist.values()
        edges = hist.axis().edges()
        ax.step(edges[:-1], values, where='pre', linewidth=2.5, 
                label=sample, color=colors[sample])
    except Exception as e:
        print(f"✗ Error plotting {sample}: {e}")

ax.set_xlabel('m_4ℓ [GeV]', fontsize=13, fontweight='bold')
ax.set_ylabel('Events / bin', fontsize=13, fontweight='bold')
ax.set_title('M2lZVeto_ZZCR: 4-Lepton Mass Overlay', fontsize=14, fontweight='bold')
#ax.set_yscale('log')
ax.set_ylim([0.01, 250])
ax.legend(fontsize=11, loc='upper right', framealpha=0.95, edgecolor='black')
ax.grid(True, alpha=0.3, linestyle='-', linewidth=0.5, which='both')

plt.tight_layout()
plt.savefig('quad_m4l_overlay.pdf', dpi=150, bbox_inches='tight')
print("\n✓ Saved: quad_m4l_overlay.pdf")
plt.show()

root_file.close()
