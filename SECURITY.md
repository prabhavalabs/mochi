# Security policy

## Report a vulnerability privately

Use [Report a vulnerability](https://github.com/prabhavalabs/mochi/security/advisories/new)
to submit a private report through GitHub. Include the affected version or commit,
steps to reproduce, expected impact, and a minimal proof of concept if available.
Do not put credentials, device backups or exploit details in a public issue.

If private reporting is temporarily unavailable, open a public issue asking for
a private reporting channel without including the vulnerability details.

Maintainers will investigate and coordinate a fix and disclosure with the reporter.
This is a volunteer project; no response-time guarantee or paid bounty is offered.

## Supported versions

Security fixes target the latest source on `main` and the current 0.2.x series.
Older development versions do not have a separate backport commitment. Update to
the latest available code before reporting an issue that may already be fixed.

## Scope and boundaries

- The portable SDK has no network client, authentication, storage or telemetry.
- Callers supply real buffer capacity and keep buffers alive for the renderer.
  The SDK validates declared layout and clips drawings; it cannot verify the
  size of the allocation behind an arbitrary pointer.
- Animator and renderer methods belong on one application task. They do not
  provide synchronization for concurrent calls.
- The example's USB console is intended for a trusted local host. Do not expose
  it through an unauthenticated network bridge.
- The browser development server binds to `127.0.0.1`. It is not a production
  hosting server. The browser preview has no external network dependencies.
- Secure boot, flash encryption and authenticated updates are not configured by
  the example. Products built with the SDK need their own security design.

Dependency vulnerabilities can affect the board adapter even when the portable
core is unaffected. Dependencies remain separately maintained; see
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## Before sharing logs or firmware

Remove credentials, personal paths and device identifiers. A full flash backup
can contain settings from an earlier application. Keep backups private and share
only minimal, redacted reproduction material.
