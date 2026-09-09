# Project governance

Mochi is an open-source project stewarded by Prabhava Labs. Contributions and
maintenance work are open to the community. This document describes the initial
governance model; it does not imply a large existing maintainer team.

## Roles

- **Contributors** propose and implement code, documentation, designs and tests.
- **Maintainers** review changes, triage issues, look after compatibility and
  dependencies, and prepare releases. Repository permissions identify the current
  people able to merge or publish.
- **Prabhava Labs repository administrators** manage access and resolve decisions
  that cannot be settled through technical review.

## Decisions

Discuss substantial changes in issues or pull requests. Explain the user problem,
tradeoffs and compatibility impact. Prefer agreement based on evidence, small
experiments and tests. A maintainer makes the final merge decision after considering
feedback; a lack of replies does not imply approval.

Changes to the public API, licensing, release process or supported boards should
have an explicit discussion and updated documentation. Maintainers can revert a
change that introduces a regression while a better solution is prepared.

## Becoming a maintainer

Regular contributors who demonstrate sound reviews, useful contributions and
respectful collaboration can ask about maintainership in an issue. Administrators
grant permissions deliberately and review them as responsibilities change. Access
should match the work being maintained.

## Releases and compatibility

The project is currently pre-1.0. Document breaking changes in the changelog and
coordinate versions across packages that ship together. Published firmware should
identify its source commit, build configuration and checksums. Before distributing
binaries, review obligations for every dependency included in the build.

Security reports follow [SECURITY.md](SECURITY.md). Participation follows the
[Code of Conduct](CODE_OF_CONDUCT.md). Maintenance is best-effort; response times
and support for older releases are not guaranteed.
