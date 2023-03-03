#pragma once
#include <QString>


class GeneralSetupArguments
{
public:
    GeneralSetupArguments();

public:
    int maxTimeout;
    int maxSet;
    int iTimeoutDuration;

    bool isPanelMirrored;

    QString sSlideDir;
    QString sSpotDir;
    QString sTeam[2];
    QString sTeamLogoFilePath[2];
};
