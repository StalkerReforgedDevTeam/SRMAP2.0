# Sickness sound set: sources and licenses

All files: 48000 Hz, mono, 16-bit PCM WAV, silence trimmed, peak normalised to -3 dBFS, 60 Hz high-pass. Nothing is synthesised; every file is a real recording taken from a public GitHub repository whose per-file attribution records a CC0 or CC-BY licence. Processing (trim, cut, resample, normalise) was done with ffmpeg + numpy; see `/tmp/claude-0/sounds_work/process.py` and `jobs.json` for exact cut points.

Repositories used (commit hashes at time of extraction):

- tgstation: https://github.com/tgstation/tgstation @ 88c08e31f21d0d001f77d538c9c333db3202ecb6. Repo rule: "All assets including icons and sound are under a CC-BY-SA 3.0 unless otherwise indicated." The files below are "otherwise indicated" in `sound/attributions.txt`, which lists the original freesound.org upload and its licence (section "modified by grungussuss"). The tgstation edit is trim/convert only. If you want to be maximally safe, download the originals from the freesound links given; the IDs are listed so you can do that in one click each.
- Space Station 14 (upstream https://github.com/space-wizards/space-station-14; extracted from the space-syndicate fork https://github.com/space-syndicate/space-station-14 @ a5e876a34b0aab47253a359e82137299f11ea17e, blob ids verified identical to upstream/other forks). Per-file licences are recorded in `Resources/Audio/.../attributions.yml`.
- Coswara dataset: https://github.com/iiscleap/Coswara-Data @ 4942c97e31de7180a93d17f2e7530a9c543cfd50. Whole repo is CC BY 4.0 (`LICENSE.md`, README). Crowdsourced respiratory recordings; every file used here is male, hand-labelled quality 2 ("excellent") in `annotations/cough-heavy_labels.csv`, and was additionally screened for noise floor, bandwidth and clipping.

## Per-file table

| Output file | Source repo | Original path in repo | Original recording | Licence | Required attribution line |
|---|---|---|---|---|---|
| cough_01.wav | tgstation | `sound/mobs/humanoids/human/cough/male_cough1.ogg` (first cough of three, cut 0.15 to 0.72 s) | freesound.org/people/Harris85/sounds/208761/ | CC0 1.0 | None required. Courtesy: "Cough by Harris85 (freesound.org), CC0" |
| cough_02.wav | tgstation | `sound/mobs/humanoids/human/cough/male_cough4.ogg` (first cough of three, cut 0.0 to 0.60 s) | freesound.org/people/SoundDesignForYou/sounds/646652/ | CC0 1.0 | None required. Courtesy: "Cough by SoundDesignForYou (freesound.org), CC0" |
| cough_03.wav | Space Station 14 | `Resources/Audio/Voice/Human/male_cough_1.ogg` | freesound.org/people/dastudiospr/sounds/537150/ | CC0 1.0 (per `Resources/Audio/Voice/Human/attributions.yml`) | None required. Courtesy: "Cough by dastudiospr (freesound.org), CC0" |
| cough_04.wav (bonus: triple cough, 1.4 s) | tgstation | `sound/mobs/humanoids/human/cough/male_cough3.ogg` (whole file) | freesound.org/people/hadescolossus/sounds/701162/ | CC0 1.0 | None required. Courtesy: "Cough by hadescolossus (freesound.org), CC0" |
| cough_05.wav (bonus: four short coughs, 1.9 s) | tgstation | `sound/mobs/humanoids/human/cough/male_cough5.ogg` (whole file) | freesound.org/people/SoundDesignForYou/sounds/646654/ | CC0 1.0 | None required. Courtesy: "Cough by SoundDesignForYou (freesound.org), CC0" |
| cough_fit_01.wav (6.9 s, 4 groups of double coughs) | Coswara-Data | `20200413/20200413.tar.gz.*` -> `20200413/EqDWckxbsETyHUeBLQ8jLtxlhir2/cough-heavy.wav` | Coswara participant EqDWckxbsETyHUeBLQ8jLtxlhir2 (male, 31, quality 2) | CC BY 4.0 | REQUIRED: "Contains audio from the Coswara dataset (Project Coswara, LEAP Lab, Indian Institute of Science, Bangalore), https://github.com/iiscleap/Coswara-Data, licensed CC BY 4.0. Edited (trimmed, normalised)." |
| cough_fit_02.wav (5.2 s, 3 heavy coughs, participant reported cough + sore throat) | Coswara-Data | `20220116/20220116.tar.gz.*` -> `20220116/sZXioPNq72VC9DKY05wQtL6MB452/cough-heavy.wav` | Coswara participant sZXioPNq72VC9DKY05wQtL6MB452 (male, quality 2) | CC BY 4.0 | Same line as cough_fit_01 (one line covers both). |
| sneeze_01.wav | tgstation | `sound/mobs/humanoids/human/sneeze/male_sneeze1.ogg` | "Sneeze, Single, A" by InspectorJ, freesound.org/people/InspectorJ/sounds/352177/ | CC BY 4.0 | REQUIRED: "Sneeze, Single, A.wav by InspectorJ (www.jshaw.co.uk) of Freesound.org, CC BY 4.0. Edited." |
| sneeze_02.wav | Space Station 14 (added via Delta-V, present in Delta-v and Corvax forks) | `Resources/Audio/Voice/Vulpkanin/vulp_sneeze.ogg` | freesound.org/people/blukotek/sounds/424949/ ("cleaned, cropped" by Minemoder5000). Used in-game for a fox-person species; the recording is a short human-sounding sneeze; the recordist's sex is not stated. | CC0 1.0 (per `Resources/Audio/Voice/Vulpkanin/attributions.yml`) | None required. Courtesy: "Sneeze by blukotek (freesound.org), CC0" |
| sneeze_03.wav | tgstation (derived) | same as sneeze_01, resampled 7% faster (slightly higher pitch, shorter) to give a third random variant. NOT a separate recording. Delete it if you only want distinct takes. | InspectorJ 352177 | CC BY 4.0 | Same line as sneeze_01. |

## Suggested combined credits block (copy into your mod's credits)

```
Sickness sound effects:
- Sneeze: "Sneeze, Single, A.wav" by InspectorJ (www.jshaw.co.uk) of Freesound.org, CC BY 4.0 (edited).
- Coughing fits: audio from the Coswara dataset, Project Coswara, LEAP Lab, Indian Institute of Science Bangalore,
  https://github.com/iiscleap/Coswara-Data, CC BY 4.0 (edited: trimmed and normalised).
- Single coughs and second sneeze: CC0 recordings by Harris85, SoundDesignForYou, hadescolossus, dastudiospr and blukotek
  (freesound.org), as redistributed with attribution in the tgstation and Space Station 14 repositories.
```

## Not delivered (nothing found under CC0 / CC-BY / MIT / PD on GitHub)

- vomit / retch: the only candidates found (tgstation `sound/mobs/humanoids/human/gag_vomit/gag1-5.ogg`, Unitystation male gags, Minetest "pathogen" mod) carry no per-file attribution and therefore fall under the repos' default CC-BY-SA 3.0 or an unspecified "CC", which does not meet the licence requirement.
- teeth chattering / shivering breath: nothing found in any searched repository.
- stomach gurgle: only "vore" mod gurgle packs (NovaSector, Bubberstation) with unclear sourcing. Not used.
