#include <QStandardPaths>

#include "generalsetuparguments.h"


GeneralSetupArguments::GeneralSetupArguments()
    : maxTimeout(2)
    , maxSet(3)
    , iTimeoutDuration(30) //In seconds
    // The default Directories to look for the slides and spots
    , sSlideDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
    , sSpotDir(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation))
{
    sTeam[0] = "Locali";
    sTeam[1] = "Ospiti";
    sTeamLogoFilePath[0] = ":/Logo_SSD_UniMe.png";
    sTeamLogoFilePath[1] = ":/Logo_UniMe.png";
}
