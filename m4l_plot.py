import uproot
import matplotlib.pyplot as plt

# Open ROOT file
root_file = uproot.open('runs/ZZCR_10Sep/final_hadded.root')

# Define histograms
histograms = {
    "ZZ_2022": "M2lZVeto_ZZCR__ZZ_2022__quad_m4l",
    "DY_2022": "M2lZVeto_ZZCR__DY_2022__quad_m4l",
    "TB_2022": "M2lZVeto_ZZCR__TB_2022__quad_m4l",
    "ttbar_2022": "M2lZVeto_ZZCR__ttbar_2022__quad_m4l",
}

# Colors
colors = {
    "ZZ_2022": "#DC143C",
    "DY_2022": "#FFA500",
    "TB_2022": "#00CED1",
    "ttbar_2022": "#228B22",
}

fig, ax = plt.subplots(figsize=(12, 8))

for sample, hist_name in histograms.items():
    hist = root_file[hist_name]
    edges = hist.axes[0].edges
    values = hist.values
    ax.step(edges[:-1], values, where='pre', linewidth=2.5, 
            label=sample, color=colors[sample])

ax.set_xlabel('m_4ℓ [GeV]', fontsize=13, fontweight='bold')
ax.set_ylabel('Events / bin', fontsize=13, fontweight='bold')
ax.set_title('M2lZVeto_ZZCR: quad_m4l Overlay', fontsize=14, fontweight='bold')
ax.set_yscale('log')
ax.legend(fontsize=11)
ax.grid(True, alpha=0.3)

plt.savefig('quad_m4l_overlay.png', dpi=150, bbox_inches='tight')
plt.show()
