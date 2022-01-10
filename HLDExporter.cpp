// HLDExporter.cpp : Définit les fonctions exportées de la DLL.
//

#include "pch.h"
#include "framework.h"
#include "HLDExporter.h"


// Il s'agit d'un exemple de variable exportée
HLDEXPORTER_API int nHLDExporter=0;

// Il s'agit d'un exemple de fonction exportée.
HLDEXPORTER_API int fnHLDExporter(void)
{
    return 0;
}

// Il s'agit du constructeur d'une classe qui a été exportée.
CHLDExporter::CHLDExporter()
{
    return;
}
