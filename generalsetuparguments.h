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

    QString sSlideDir;
    QString sSpotDir;
    QString sTeam0Name;
    QString sTeam1Name;
    QString sTeam0LogoFilePath;
    QString sTeam1LogoFilePath;
};
