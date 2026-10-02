# Taste

- Prefers deliverables (config/ops artifacts like service units) materialized as real files committed into the repo with accompanying docs, rather than only presented as chat answers. Confidence: 0.5
- Expects technical claims/instructions (especially in generated docs) to be verified against the actual environment/hardware rather than asserted from assumption, and will push back when they don't hold. Confidence: 0.5
- Wants sibling/parallel components kept in sync: when a change or capability is added to one program (e.g. `scanner`), expects it applied to its counterpart too (e.g. `doublescanner`) rather than leaving them divergent. Confidence: 0.5
- Wants documentation (e.g. the README feature checklist and intro prose) kept in sync with the actual implemented feature set — expects docs updated when functionality lands or changes. Confidence: 0.5
