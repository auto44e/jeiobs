# Security policy

This policy covers **JeiOBS (based on OBS Studio)**, the independent fork in this
repository. It is not the OBS Project's security program, and the OBS Project does
not handle JeiOBS reports. See the [fork notice](docs/jeiobs/NOTICE.md).

JeiOBS is pre-release development software and is used at your own risk. It is not
a supported production build.

## Reporting a vulnerability

Report privately through GitHub: open the **Security** tab of this repository and
choose **Report a vulnerability**
(<https://github.com/auto44e/jeiobs/security/advisories/new>).

Please do not open a public issue for something that could be exploited. Include
the affected version or commit, the platform, clear reproduction steps (a short
proof of concept helps), and the impact you see.

## What to expect

JeiOBS is maintained by a small group on a best-effort basis. There is no
guaranteed response time, no fix deadline and no bug bounty. We will try to confirm
reports, fix real issues, and credit reporters who want credit. Please keep details
confidential until a fix or a mutually agreed disclosure date.

The OBS Project's program terms (response times, safe harbor, disclosure period)
are upstream's and do not apply to JeiOBS.

## Good-faith research

Test only on systems you own or are authorized to test. Avoid violating anyone's
privacy, destroying data or degrading others' service, do not run automated scans
against infrastructure that is not yours, and do not use a report to demand
payment.

## Upstream code

If the problem exists in code that is unchanged from upstream OBS Studio, please
also report it through upstream's own channels, described in
[their security policy](https://github.com/obsproject/obs-studio/blob/master/SECURITY.md).
Send JeiOBS-specific issues only here.
