/****************************************************************************
* MeshLab                                                           o o     *
* A versatile mesh processing toolbox                             o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/
#include "filter_create.h"
#include <vcg/complex/algorithms/create/platonic.h>
#include <vcg/complex/algorithms/point_sampling.h>

// Constructor usually performs only two simple tasks of filling the two lists
//  - typeList: with all the possible id of the filtering actions
//  - actionList with the corresponding actions. If you want to add icons to your filtering actions you can do here by construction the QActions accordingly

FilterCreate::FilterCreate()
{
    typeList <<CR_BOX<< CR_ANNULUS << CR_SPHERE<< CR_RANDOM_SPHERE<< CR_ICOSAHEDRON<< CR_DODECAHEDRON<< CR_TETRAHEDRON<<CR_OCTAHEDRON<<CR_CONE<<CR_TORUS<<CR_PLANE;

    foreach(FilterIDType tt , types())
        actionList << new QAction(filterName(tt), this);
}

QString FilterCreate::filterName(FilterIDType filterId) const
{
    switch(filterId) {
        case CR_BOX : return QString("Box");
        case CR_ANNULUS : return QString("Annulus");
        case CR_SPHERE: return QString("Sphere");
        case CR_RANDOM_SPHERE: return QString("Random Sphere");
        case CR_ICOSAHEDRON: return QString("Icosahedron");
        case CR_DODECAHEDRON: return QString("Dodecahedron");
        case CR_OCTAHEDRON: return QString("Octahedron");
        case CR_TETRAHEDRON: return QString("Tetrahedron");
        case CR_CONE: return QString("Cone");
        case CR_TORUS: return QString("Torus");
        case CR_PLANE: return QString("Plane");
        default : assert(0);
    }
}

// Info() must return the longer string describing each filtering action
// (this string is used in the About plugin dialog)
QString FilterCreate::filterInfo(FilterIDType filterId) const
{
    switch(filterId) {
        case CR_BOX : return QString("Create a Box");
        case CR_ANNULUS : return QString("Create an Annulus, e.g. a flat region bounded by two concentric circles");
        case CR_SPHERE: return QString("Create a Sphere");
        case CR_RANDOM_SPHERE: return QString("Create a random Spherical point cloud");
        case CR_ICOSAHEDRON: return QString("Create an Icosahedron");
        case CR_DODECAHEDRON: return QString("Create an Dodecahedron");
        case CR_OCTAHEDRON: return QString("Create an Octahedron");
        case CR_TETRAHEDRON: return QString("Create a Tetrahedron");
        case CR_CONE: return QString("Create a Cone");
        case CR_TORUS: return QString("Create a Torus");
        case CR_PLANE: return QString("Create a Plane");
        default : assert(0);
    }
}

// This function define the needed parameters for each filter. Return true if the filter has some parameters
// it is called every time, so you can set the default value of parameters according to the mesh
// For each parmeter you need to define,
// - the name of the parameter,
// - the string shown in the dialog
// - the default value
// - a possibly long string describing the meaning of that parameter (shown as a popup help in the dialog)
void FilterCreate::initParameterSet(QAction *action, MeshModel & /*m*/, RichParameterSet & parlst)
{
    switch(ID(action))
    {
        case CR_SPHERE :
            parlst.addParam(new RichFloat("radius",1,"Radius","Radius of the sphere"));
            parlst.addParam(new RichInt("subdiv",3,"Subdiv. Level","Number of the recursive subdivision of the surface. Default is 3 (a sphere approximation composed by 1280 faces).<br>"
                                        "Admitted values are in the range 0 (an icosahedron) to 8 (a 1.3 MegaTris approximation of a sphere)"));
            break;
        case CR_ANNULUS :
            parlst.addParam(new RichFloat("internalRadius",0.5f,"Internal Radius","Internal Radius of the annulus"));
            parlst.addParam(new RichFloat("externalRadius",1.0f,"External Radius","Externale Radius of the annulus"));
            parlst.addParam(new RichInt("sides",32,"Sides","Number of the sides of the poligonal approximation of the annulus "));
            break;
        case CR_RANDOM_SPHERE :
            parlst.addParam(new RichInt("pointNum",100,"Point Num","Number of points (approximate)."));
            break;
        case CR_BOX :
            parlst.addParam(new RichFloat("size",1,"Scale factor","Scales the new mesh"));
            break;
        case CR_CONE:
            parlst.addParam(new RichFloat("r0",1,"Radius 1","Radius of the bottom circumference"));
            parlst.addParam(new RichFloat("r1",2,"Radius 2","Radius of the top circumference"));
            parlst.addParam(new RichFloat("h",3,"Height","Height of the Cone"));
            parlst.addParam(new RichInt("subdiv",36,"Side","Number of sides of the polygonal approximation of the cone"));
            break;
        case CR_TORUS:
            parlst.addParam(new RichFloat("hRadius",3,"Horizontal Radius","Radius of the whole horizontal ring of the torus"));
            parlst.addParam(new RichFloat("vRadius",1,"Vertical Radius","Radius of the vertical section of the ring"));
            parlst.addParam(new RichInt("hSubdiv",24,"Horizontal Subdivision","Subdivision step of the ring"));
            parlst.addParam(new RichInt("vSubdiv",12,"Vertical Subdivision","Number of sides of the polygonal approximation of the torus section"));
            break;
        case CR_PLANE:
        {
            Eigen::Matrix<float,4,4> corners_eig;
            corners_eig << 0.f,0.f,1.f,1.f,
                    0.f,1.f,0.f,1.f,
                    0.f,0.f,0.f,0.f,
                    1.f,1.f,1.f,1.f;
            vcg::Matrix44f corners;
            corners.FromEigenMatrix( corners_eig );
            parlst.addParam( new RichMatrix44f("corners",corners,"Corners in columns","4 corner points in columns") );
            //   RichMatrix44f(const QString nm,const vcg::Matrix44f& defval,const QString desc=QString(),const QString tltip=QString());
            break;
        }
        default:
            return;
            break;
    }
}

// The Real Core Function doing the actual mesh processing.
bool FilterCreate::applyFilter(QAction *filter, MeshDocument &md, RichParameterSet & par, vcg::CallBackPos * /*cb*/)
{
    MeshModel* m=md.addNewMesh("",this->filterName(ID(filter)));
    switch(ID(filter))
    {
        case CR_TETRAHEDRON :
            vcg::tri::Tetrahedron<CMeshO>(m->cm);
            break;
        case CR_ICOSAHEDRON:
            vcg::tri::Icosahedron<CMeshO>(m->cm);
            break;
        case CR_DODECAHEDRON:
            vcg::tri::Dodecahedron<CMeshO>(m->cm);
            m->updateDataMask(MeshModel::MM_POLYGONAL);
            break;
        case CR_OCTAHEDRON:
            vcg::tri::Octahedron<CMeshO>(m->cm);
            break;
        case CR_ANNULUS:
            vcg::tri::Annulus<CMeshO>(m->cm,par.getFloat("internalRadius"),
                                      par.getFloat("externalRadius"),par.getInt("sides"));
            break;
        case CR_TORUS:
        {
            float hRadius=par.getFloat("hRadius");
            float vRadius=par.getFloat("vRadius");
            int hSubdiv=par.getInt("hSubdiv");
            int vSubdiv=par.getInt("vSubdiv");
            vcg::tri::Torus(m->cm,hRadius,vRadius,hSubdiv,vSubdiv);
            break;
        }
        case CR_RANDOM_SPHERE:
        {
            CMeshO tt;

            int pointNum = par.getInt("pointNum");
            int oversamplingFactor =100;
            if(pointNum <= 100) oversamplingFactor = 1000;
            if(pointNum >= 10000) oversamplingFactor = 50;
            if(pointNum >= 100000) oversamplingFactor = 20;

            vcg::math::MarsenneTwisterRNG rng;
            vcg::tri::Allocator<CMeshO>::AddVertices(tt,pointNum*50);
            for(CMeshO::VertexIterator vi=tt.vert.begin();vi!=tt.vert.end();++vi)
                vi->P()=vcg::math::GeneratePointOnUnitSphereUniform<float>(rng);
            vcg::tri::UpdateBounding<CMeshO>::Box(tt);

            const float SphereArea = 4*M_PI;
            float poissonRadius = 2.0*sqrt((SphereArea / float(pointNum*2))/M_PI);

            std::vector<vcg::Point3f> poissonSamples;
            vcg::tri::TrivialSampler<CMeshO> pdSampler(poissonSamples);
            vcg::tri::SurfaceSampling<CMeshO, vcg::tri::TrivialSampler<CMeshO> >::PoissonDiskParam pp;

            vcg::tri::SurfaceSampling<CMeshO,vcg::tri::TrivialSampler<CMeshO> >::PoissonDiskPruning(pdSampler, tt, poissonRadius, pp);
            m->cm.Clear();
            vcg::tri::Allocator<CMeshO>::AddVertices(m->cm,poissonSamples.size());
            for(size_t i=0;i<poissonSamples.size();++i)
            {
                m->cm.vert[i].P()=poissonSamples[i];
                m->cm.vert[i].N()=m->cm.vert[i].P();
            }
        } break;

        case CR_SPHERE:
        {
            int rec = par.getInt("subdiv");
            float radius = par.getFloat("radius");
            m->cm.face.EnableFFAdjacency();
            m->updateDataMask(MeshModel::MM_FACEFACETOPO);
            assert(vcg::tri::HasPerVertexTexCoord(m->cm) == false);
            vcg::tri::Sphere<CMeshO>(m->cm,rec);

            for(CMeshO::VertexIterator vi = m->cm.vert.begin();vi!= m->cm.vert.end();++vi)
                vi->P()=vi->P()*radius;

            break;
        }
        case CR_BOX:
        {
            float sz=par.getFloat("size");
            vcg::Box3f b(vcg::Point3f(1,1,1)*(-sz/2),vcg::Point3f(1,1,1)*(sz/2));
            vcg::tri::Box<CMeshO>(m->cm,b);
            m->updateDataMask(MeshModel::MM_POLYGONAL);

            break;
        }
        case CR_PLANE:
        {
            //vcg::tri::Plane<CMeshO>(m->cm,b);
            typedef CMeshO                              MeshType;
            typedef typename MeshType::ScalarType       ScalarType;
            typedef typename MeshType::CoordType        CoordType;
            typedef typename MeshType::VertexPointer    VertexPointer;
            typedef typename MeshType::VertexIterator   VertexIterator;
            typedef typename MeshType::FaceIterator     FaceIterator;

            // acquire
            vcg::Matrix44f corners = par.getMatrix44( "corners" );

//            bool parse_success = true;
//            QClipboard *clipboard = QApplication::clipboard();
//            if ( clipboard )
//            {
//                qDebug() << "clipboard: " << qPrintable( clipboard->text() );
//                QString clipboard_text = clipboard->text();
//                QStringList word_list = clipboard_text.split(" ");
//                QStringList::const_iterator it_end = word_list.end();
//                int word_id = 0;
//                for ( QStringList::const_iterator it = word_list.begin(); (it != it_end) && (word_id != 16); ++it )
//                {
//                    qDebug() << "word: " << qPrintable( *it ) << "\t";
//                    corners.ElementAt( word_id / 4, word_id %4 ) = (*it).toFloat();
//                    ++word_id;
//                }
//            }

            MeshType &in = m->cm;

            in.Clear();
            vcg::tri::Allocator<MeshType>::AddVertices( in, 4 );
            vcg::tri::Allocator<MeshType>::AddFaces   ( in, 2 );

            VertexPointer ivp[4];
            VertexIterator vi=in.vert.begin();
            ivp[0] = &*vi; (*vi).P() = CoordType( corners.ElementAt(0,0), corners.ElementAt(1,0), corners.ElementAt(2,0) ); ++vi;
            ivp[1] = &*vi; (*vi).P() = CoordType( corners.ElementAt(0,1), corners.ElementAt(1,1), corners.ElementAt(2,1) ); ++vi;
            ivp[2] = &*vi; (*vi).P() = CoordType( corners.ElementAt(0,3), corners.ElementAt(1,3), corners.ElementAt(2,3) ); ++vi;
            ivp[3] = &*vi; (*vi).P() = CoordType( corners.ElementAt(0,2), corners.ElementAt(1,2), corners.ElementAt(2,2) ); ++vi;

            FaceIterator fi = in.face.begin();
            //(*fi).V(0) = ivp[2];  (*fi).V(1) = ivp[1]; (*fi).V(2) = ivp[0]; ++fi;
            //(*fi).V(0) = ivp[1];  (*fi).V(1) = ivp[2]; (*fi).V(2) = ivp[3]; ++fi;
            (*fi).V(0) = ivp[0];  (*fi).V(1) = ivp[1]; (*fi).V(2) = ivp[2]; ++fi;
            (*fi).V(0) = ivp[3];  (*fi).V(1) = ivp[2]; (*fi).V(2) = ivp[1]; ++fi;

            if ( HasPerFaceFlags(in) )
            {
                FaceIterator fi = in.face.begin();
                for ( int k = 0; k != 12; ++k, ++fi )   (*fi).SetF(1);
            }

            /*vcg::Matrix44f sz=par.getFloat("corners");
            vcg::Box3f b(vcg::Point3f(1,1,1)*(-sz/2),vcg::Point3f(1,1,1)*(sz/2));
            vcg::tri::Box<CMeshO>(m->cm,b);*/

            m->updateDataMask(MeshModel::MM_POLYGONAL);
            break;
        }
        case CR_CONE:
            float r0=par.getFloat("r0");
            float r1=par.getFloat("r1");
            float h=par.getFloat("h");
            int subdiv=par.getInt("subdiv");
            vcg::tri::Cone<CMeshO>(m->cm,r0,r1,h,subdiv);
            break;
    }
    vcg::tri::UpdateBounding<CMeshO>::Box(m->cm);
    vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFaceNormalized(m->cm);
    return true;


}

MeshFilterInterface::FilterClass FilterCreate::getClass(QAction *a)
{
    switch(ID(a))
    {

        case CR_BOX:
        case CR_TETRAHEDRON:
        case CR_ICOSAHEDRON:
        case CR_DODECAHEDRON:
        case CR_SPHERE:
        case CR_ANNULUS:
        case CR_RANDOM_SPHERE:
        case CR_OCTAHEDRON:
        case CR_CONE:
        case CR_TORUS:
        case CR_PLANE:
            return MeshFilterInterface::MeshCreation;
            break;
        default: assert(0);
            return MeshFilterInterface::Generic;
    }
}

QString FilterCreate::filterScriptFunctionName( FilterIDType filterID )
{
    switch(filterID)
    {
        case CR_BOX : return QString("box");
        case CR_ANNULUS : return QString("annulus");
        case CR_SPHERE: return QString("sphere");
        case CR_RANDOM_SPHERE: return QString("randomsphere");
        case CR_ICOSAHEDRON: return QString("icosahedron");
        case CR_DODECAHEDRON: return QString("dodecahedron");
        case CR_OCTAHEDRON: return QString("octahedron");
        case CR_TETRAHEDRON: return QString("tetrahedron");
        case CR_CONE: return QString("cone");
        case CR_TORUS: return QString("torus");
        case CR_PLANE: return QString("plane");
        default : assert(0);
    }
}


MESHLAB_PLUGIN_NAME_EXPORTER(FilterCreate)
