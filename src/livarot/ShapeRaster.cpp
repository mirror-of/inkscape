/*
 *  ShapeRaster.cpp
 *  nlivarot
 *
 *  Created by fred on Sat Jul 19 2003.
 *
 */

#include "Shape.h"
#include "LivarotDefs.h"

#include "Ligne.h"
#include "AlphaLigne.h"
#include "BitLigne.h"

#include "../libnr/nr-types.h"
#include "../libnr/nr-point-fns.h"

/*
 * polygon rasterization: the sweepline algorithm in all its glory
 * nothing unusual in this implementation, so nothing special to say
 * the *Quick*() functions are not useful. forget about them
 */

void              Shape::BeginRaster(float &pos,int &curPt,float /*step*/)
{
	if ( nbPt <= 1 || nbAr <= 1 ) {
		curPt=0;
		pos=0;
		return;
	}
	MakeRasterData(true);
	MakePointData(true);
	MakeEdgeData(true);
	if ( GetFlag(has_sweep_data) ) {
	} else {
SweepTree::CreateList(sTree,nbAr);
SweepEvent::CreateQueue(sEvts,nbAr);
		SetFlag(has_sweep_data,true);
	}

	SortPoints();

	curPt=0;
	pos=pts[0].x[1]-1.0;

	for (int i=0;i<nbPt;i++) {
		pData[i].pending=0;
		pData[i].edgeOnLeft=-1;
		pData[i].nextLinkedPoint=-1;
		pData[i].rx[0]=/*Round(*/pts[i].x[0]/*)*/;
		pData[i].rx[1]=/*Round(*/pts[i].x[1]/*)*/;
	}
	for (int i=0;i<nbAr;i++) {
		eData[i].rdx=pData[aretes[i].en].rx-pData[aretes[i].st].rx;
	}
}
void              Shape::EndRaster(void)
{
	if ( GetFlag(has_sweep_data) ) {
SweepTree::DestroyList(sTree);
SweepEvent::DestroyQueue(sEvts);
		SetFlag(has_sweep_data,false);
	}
	MakePointData(false);
	MakeEdgeData(false);
	MakeRasterData(false);
}
void              Shape::BeginQuickRaster(float &pos,int &curPt,float /*step*/)
{
	if ( nbPt <= 1 || nbAr <= 1 ) {
		curPt=0;
		pos=0;
		return;
	}
	MakeRasterData(true);
	MakeQuickRasterData(true);
	nbQRas=0;
	MakePointData(true);
	MakeEdgeData(true);

	curPt=0;
	pos=pts[0].x[1]-1.0;

	for (int i=0;i<nbPt;i++) {
		pData[i].pending=0;
		pData[i].edgeOnLeft=-1;
		pData[i].nextLinkedPoint=-1;
		pData[i].rx[0]=Round(pts[i].x[0]);
		pData[i].rx[1]=Round(pts[i].x[1]);
	}
	for (int i=0;i<nbAr;i++) {
		eData[i].rdx=pData[aretes[i].en].rx-pData[aretes[i].st].rx;
	}
	SortPoints();
//	SortPointsRounded();
}
void              Shape::EndQuickRaster(void)
{
	MakePointData(false);
	MakeEdgeData(false);
	MakeRasterData(false);
	MakeQuickRasterData(false);
}

// 2 versions of the Scan() series to move the scanline to a given position withou actually computing coverages
void              Shape::Scan(float &pos,int &curP,float to,float step)
{
	if ( pos == to ) return;
	if ( pos < to ) {
    // we're moving downwards
    // points of the polygon are sorted top-down, so we take them in order, starting with the one at index curP,
    // until we reach the wanted position to.
    // don't forget to update curP and pos when we're done
		int    curPt=curP;
		while ( curPt < nbPt && pts[curPt].x[1] <= to ) {
			int           nPt=-1;
			nPt=curPt++;

      // treat a new point: remove and add edges incident to it
			int    cb;
			int    nbUp=0,nbDn=0;
			int    upNo=-1,dnNo=-1;
			cb=pts[nPt].firstA;
      // count the number of edge coming in nPt from above if nbUp, and the number of edge exiting nPt to go below in nbDn
      // upNo and dnNo are one of these edges, if any exist
			while ( cb >= 0 && cb < nbAr ) {
				if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
					upNo=cb;
					nbUp++;
				}
				if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
					dnNo=cb;
					nbDn++;
				}
				cb=NextAt(nPt,cb);
			}

			if ( nbDn <= 0 ) {
				upNo=-1;
			}
			if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
				upNo=-1;
			}

			if ( nbUp > 0 ) {
        // first remove edges coming from above
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != upNo ) { // we salvage the edge upNo to plug the edges we'll be addingat its place
                                // but the other edge don't have this chance
							SweepTree* node=swrData[cb].misc;
							if ( node ) {
								swrData[cb].misc=NULL;
								node->Remove(sTree,sEvts,true);
								DestroyEdge(cb,to,step);
							}
						}
					}
					cb=NextAt(nPt,cb);
				}
			}

			// if there is one edge going down and one edge coming from above, we don't Insert() the new edge,
      // but replace the upNo edge by the new one (faster)
			SweepTree* insertionNode=NULL;
			if ( dnNo >= 0 ) {
				if ( upNo >= 0 ) {
					SweepTree* node=swrData[upNo].misc;
					swrData[upNo].misc=NULL;
					DestroyEdge(upNo,to,step);

					node->ConvertTo(this,dnNo,1,nPt);

					swrData[dnNo].misc=node;
					insertionNode=node;
					CreateEdge(dnNo,to,step);
				} else {
					SweepTree* node=SweepTree::AddInList(this,dnNo,1,nPt,sTree,this);
					swrData[dnNo].misc=node;
					node->Insert(sTree,sEvts,this,nPt,true);
					insertionNode=node;
					CreateEdge(dnNo,to,step);
				}
			}

      // add the remaining edges
			if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != dnNo ) {
							SweepTree* node=SweepTree::AddInList(this,cb,1,nPt,sTree,this);
							swrData[cb].misc=node;
							node->InsertAt(sTree,sEvts,this,insertionNode,nPt,true);
							CreateEdge(cb,to,step);
						}
					}
					cb=NextAt(nPt,cb);
				}
			}
		}
		curP=curPt;
		if ( curPt > 0 ) pos=pts[curPt-1].x[1]; else pos=to;
	} else {
    // same thing, but going up. so the sweepSens is inverted for the Find() function
		int    curPt=curP;
		while ( curPt > 0 && pts[curPt-1].x[1] >= to ) {
			int           nPt=-1;
			nPt=--curPt;

			int    cb;
			int    nbUp=0,nbDn=0;
			int    upNo=-1,dnNo=-1;
			cb=pts[nPt].firstA;
			while ( cb >= 0 && cb < nbAr ) {
				if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
					upNo=cb;
					nbUp++;
				}
				if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
					dnNo=cb;
					nbDn++;
				}
				cb=NextAt(nPt,cb);
			}

			if ( nbDn <= 0 ) {
				upNo=-1;
			}
			if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
				upNo=-1;
			}

			if ( nbUp > 0 ) {
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != upNo ) {
							SweepTree* node=swrData[cb].misc;
							if ( node ) {
								swrData[cb].misc=NULL;
								node->Remove(sTree,sEvts,true);
								DestroyEdge(cb,to,step);
							}
						}
					}
					cb=NextAt(nPt,cb);
				}
			}

			// traitement du "upNo devient dnNo"
			SweepTree* insertionNode=NULL;
			if ( dnNo >= 0 ) {
				if ( upNo >= 0 ) {
					SweepTree* node=swrData[upNo].misc;
					swrData[upNo].misc=NULL;
					DestroyEdge(upNo,to,step);

					node->ConvertTo(this,dnNo,1,Other(nPt,dnNo));

					swrData[dnNo].misc=node;
					insertionNode=node;
					CreateEdge(dnNo,to,step);
				} else {
					SweepTree* node=SweepTree::AddInList(this,dnNo,1,nPt,sTree,this);
					swrData[dnNo].misc=node;
					node->Insert(sTree,sEvts,this,nPt,true,false);
					node->startPoint=Other(nPt,dnNo);
					insertionNode=node;
					CreateEdge(dnNo,to,step);
				}
			}

			if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != dnNo ) {
							SweepTree* node=SweepTree::AddInList(this,cb,1,nPt,sTree,this);
							swrData[cb].misc=node;
							node->InsertAt(sTree,sEvts,this,insertionNode,nPt,true,false);
							node->startPoint=Other(nPt,cb);
							CreateEdge(cb,to,step);
						}
					}
					cb=NextAt(nPt,cb);
				}
			}
		}
		curP=curPt;
		if ( curPt > 0 ) pos=pts[curPt-1].x[1]; else pos=to;
	}
  // the final touch: edges intersecting the sweepline must be update so that their intersection with
  // said sweepline is correct.
	if ( sTree.racine ) {
		SweepTree* curS=static_cast <SweepTree*> (sTree.racine->Leftmost());
		while ( curS ) {
			int    cb=curS->bord;
			AvanceEdge(cb,to,true,step);
			curS=static_cast <SweepTree*> (curS->rightElem);
		}
	}
}

void              Shape::QuickScan(float &pos,int &curP,float to,bool doSort,float step)
{
	if ( pos == to ) return;
	if ( pos < to ) {
		int    curPt=curP;
		while ( curPt < nbPt && pts[curPt].x[1] <= to ) {
			int           nPt=-1;
			nPt=curPt++;

			int    cb;
			int    nbUp=0,nbDn=0;
			int    upNo=-1,dnNo=-1;
			cb=pts[nPt].firstA;
			while ( cb >= 0 && cb < nbAr ) {
				if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
					upNo=cb;
					nbUp++;
				}
				if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
					dnNo=cb;
					nbDn++;
				}
				cb=NextAt(nPt,cb);
			}

			if ( nbDn <= 0 ) {
				upNo=-1;
			}
/*			if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
				upNo=-1;
			}*/

			if ( nbUp > 0 ) {
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != upNo ) {
							int ind=qrsData[cb].ind;
							qrsData[ind].bord=qrsData[--nbQRas].bord;
							if ( nbQRas > 0 ) qrsData[qrsData[ind].bord].ind=ind;

							DestroyEdge(cb,to,step);
						}
					}
					cb=NextAt(nPt,cb);
				}
			}

			// traitement du "upNo devient dnNo"
			if ( dnNo >= 0 ) {
				if ( upNo >= 0 ) {
					int ind=qrsData[upNo].ind;
					qrsData[ind].bord=dnNo;
					qrsData[dnNo].ind=ind;
					DestroyEdge(upNo,to,step);

					CreateEdge(dnNo,to,step);
				} else {
					int ind=nbQRas++;
					qrsData[ind].bord=dnNo;
					qrsData[dnNo].ind=ind;
					CreateEdge(dnNo,to,step);
				}
			}

			if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != dnNo ) {
							int ind=nbQRas++;
							qrsData[ind].bord=cb;
							qrsData[cb].ind=ind;
							CreateEdge(cb,to,step);
						}
					}
					cb=NextAt(nPt,cb);
				}
			}
		}
		curP=curPt;
		if ( curPt > 0 ) pos=pts[curPt-1].x[1]; else pos=to;
	} else {
		int    curPt=curP;
		while ( curPt > 0 && pts[curPt-1].x[1] >= to ) {
			int           nPt=-1;
			nPt=--curPt;

			int    cb;
			int    nbUp=0,nbDn=0;
			int    upNo=-1,dnNo=-1;
			cb=pts[nPt].firstA;
			while ( cb >= 0 && cb < nbAr ) {
				if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
					upNo=cb;
					nbUp++;
				}
				if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
					dnNo=cb;
					nbDn++;
				}
				cb=NextAt(nPt,cb);
			}

			if ( nbDn <= 0 ) {
				upNo=-1;
			}
/*			if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
				upNo=-1;
			}*/

			if ( nbUp > 0 ) {
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != upNo ) {
							int ind=qrsData[cb].ind;
							qrsData[ind].bord=qrsData[--nbQRas].bord;
							if ( nbQRas > 0 ) qrsData[qrsData[ind].bord].ind=ind;
							DestroyEdge(cb,to,step);
						}
					}
					cb=NextAt(nPt,cb);
				}
			}

			// traitement du "upNo devient dnNo"
			if ( dnNo >= 0 ) {
				if ( upNo >= 0 ) {
					int ind=qrsData[upNo].ind;
					qrsData[ind].bord=dnNo;
					qrsData[dnNo].ind=ind;
					DestroyEdge(upNo,to,step);

					CreateEdge(dnNo,to,step);
				} else {
					int ind=nbQRas++;
					qrsData[ind].bord=dnNo;
					qrsData[dnNo].ind=ind;
					CreateEdge(dnNo,to,step);
				}
			}

			if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != dnNo ) {
							int ind=nbQRas++;
							qrsData[ind].bord=cb;
							qrsData[cb].ind=ind;
							CreateEdge(cb,to,step);
						}
					}
					cb=NextAt(nPt,cb);
				}
			}
		}
		curP=curPt;
		if ( curPt > 0 ) pos=pts[curPt-1].x[1]; else pos=to;
	}
	for (int i=0;i<nbQRas;i++) {
		int cb=qrsData[i].bord;
		AvanceEdge(cb,to,true,step);
		qrsData[i].x=swrData[cb].curX;
	}
	if ( nbQRas > 1 && doSort) {
		qsort(qrsData,nbQRas,sizeof(quick_raster_data),CmpQuickRaster);
		for (int i=0;i<nbQRas;i++) qrsData[qrsData[i].bord].ind=i;
	}
}
// scan and compute coverage, FloatLigne version
// coverage of the line is bult in 2 parts: first a set of rectangles of height the height of the line (here: "step")
// one rectangle for each portion of the sweepline that is in the polygon at the beginning of the scan. then a set ot trapezoids
// are added or removed to these rectangles, one trapezoid for each edge destroyed or edge crossing the entire line. think of
// it as a refinement of the coverage by rectangles
void              Shape::Scan(float &pos,int &curP,float to,FloatLigne* line,bool exact,float step)
{
	if ( pos >= to ) return;
	if ( pos < to ) {
    // first step: the rectangles
    // since we read the sweepline left to right, we know the boundaries of the rectangles are appended in a list, hence
    // the AppendBord(). we salvage the guess value for the trapezoids the edges will induce
 		if ( sTree.racine ) {
			SweepTree* curS=static_cast <SweepTree*> (sTree.racine->Leftmost());
			while ( curS ) {
				int    lastGuess=-1;
				int    cb=curS->bord;
				//				printf("%i %i %f/ ",cb,(swrData[cb].sens)?1:0,swrData[cb].curX);
				if ( swrData[cb].sens == false && curS->leftElem ) {
					int  lb=(static_cast <SweepTree*> (curS->leftElem))->bord;
					lastGuess=line->AppendBord(swrData[lb].curX,to-swrData[lb].curY,swrData[cb].curX,to-swrData[cb].curY,0.0);
					swrData[lb].guess=lastGuess-1;
					swrData[cb].guess=lastGuess;
				} else {
					int  lb=curS->bord;
					swrData[lb].guess=-1;
        }
				curS=static_cast <SweepTree*> (curS->rightElem);
			}
			//			printf("\n");
		}
		int    curPt=curP;
		while ( curPt < nbPt && pts[curPt].x[1] <= to ) {
			int           nPt=-1;
			nPt=curPt++;

      // same thing as the usual Scan(), just with a hardcoded "indegree+outdegree=2" case, since it's the most common one
			int    cb;
			int    nbUp=0,nbDn=0;
			int    upNo=-1,dnNo=-1;
			if ( pts[nPt].dI+pts[nPt].dO == 2 ) {
				cb=pts[nPt].firstA;
				if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
					upNo=cb;
					nbUp++;
				}
				if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
					dnNo=cb;
					nbDn++;
				}
				cb=pts[nPt].lastA;
				if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
					upNo=cb;
					nbUp++;
				}
				if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
					dnNo=cb;
					nbDn++;
				}

			} else {
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
						upNo=cb;
						nbUp++;
					}
					if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
						dnNo=cb;
						nbDn++;
					}
					cb=NextAt(nPt,cb);
				}
			}

			if ( nbDn <= 0 ) {
				upNo=-1;
			}
			if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
				upNo=-1;
			}

			if ( nbUp > 1 || ( nbUp == 1 && upNo < 0 ) ) {
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != upNo ) {
							SweepTree* node=swrData[cb].misc;
							if ( node ) {
								swrData[cb].lastX=swrData[cb].curX;
								swrData[cb].lastY=swrData[cb].curY;
								swrData[cb].curX=pts[nPt].x[0];
								swrData[cb].curY=pts[nPt].x[1];
								swrData[cb].misc=NULL;
								DestroyEdge(cb,to,line,step); // create trapezoid for the chunk of edge intersecting with the line

								node->Remove(sTree,sEvts,true);
							}
						}
					}
					cb=NextAt(nPt,cb);
				}
			}

			// traitement du "upNo devient dnNo"
			SweepTree* insertionNode=NULL;
			if ( dnNo >= 0 ) {
				if ( upNo >= 0 ) {
					SweepTree* node=swrData[upNo].misc;
					swrData[upNo].misc=NULL;
					swrData[upNo].lastX=swrData[upNo].curX;
					swrData[upNo].lastY=swrData[upNo].curY;
					swrData[upNo].curX=pts[nPt].x[0];
					swrData[upNo].curY=pts[nPt].x[1];
					DestroyEdge(upNo,to,line,step);

					node->ConvertTo(this,dnNo,1,nPt);

					swrData[dnNo].misc=node;
					insertionNode=node;
					CreateEdge(dnNo,to,step);
					swrData[dnNo].guess=swrData[upNo].guess;
				} else {
					SweepTree* node=SweepTree::AddInList(this,dnNo,1,nPt,sTree,this);
					swrData[dnNo].misc=node;
					node->Insert(sTree,sEvts,this,nPt,true);
					insertionNode=node;
					CreateEdge(dnNo,to,step);
				}
			}

			if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != dnNo ) {
							SweepTree* node=SweepTree::AddInList(this,cb,1,nPt,sTree,this);
							swrData[cb].misc=node;
							node->InsertAt(sTree,sEvts,this,insertionNode,nPt,true);
							CreateEdge(cb,to,step);
						}
					}
					cb=NextAt(nPt,cb);
				}
			}
		}
		curP=curPt;
		if ( curPt > 0 ) pos=pts[curPt-1].x[1]; else pos=to;
	}
  // update intersections with the sweepline, and add trapezoids for edges crossing the line
	if ( sTree.racine ) {
		SweepTree* curS=static_cast <SweepTree*> (sTree.racine->Leftmost());
		while ( curS ) {
			int    cb=curS->bord;
			AvanceEdge(cb,to,line,exact,step);
			curS=static_cast <SweepTree*> (curS->rightElem);
		}
	}
}
void              Shape::Scan(float &pos,int &curP,float to,FillRule directed,BitLigne* line,bool exact,float step)
{
	if ( pos >= to ) return;
	if ( pos < to ) {
		if ( sTree.racine ) {
			int curW=0;
			float  lastX=0;
			SweepTree* curS=static_cast <SweepTree*> (sTree.racine->Leftmost());
			if ( directed == fill_oddEven ) {
				while ( curS ) {
					int    cb=curS->bord;
					//				printf("%i %i %f/ ",cb,(swrData[cb].sens)?1:0,swrData[cb].curX);
						//					int   oW=curW;
					curW++;
					curW&=0x00000001;
					if ( curW == 0 ) {
						line->AddBord(lastX,swrData[cb].curX,true);
					} else {
						lastX=swrData[cb].curX;
					}
					curS=static_cast <SweepTree*> (curS->rightElem);
				}
			} else if ( directed == fill_positive ) {
				// doesn't behave correctly; no way i know to do this without a ConvertToShape()
				while ( curS ) {
					int    cb=curS->bord;
					//				printf("%i %i %f/ ",cb,(swrData[cb].sens)?1:0,swrData[cb].curX);
						int   oW=curW;
					if ( swrData[cb].sens ) curW++; else curW--;

					if ( curW <= 0 && oW > 0) {
						line->AddBord(lastX,swrData[cb].curX,true);
					} else if ( curW > 0 && oW <= 0 ) {
						lastX=swrData[cb].curX;
					}
					curS=static_cast <SweepTree*> (curS->rightElem);
				}
			} else if ( directed == fill_nonZero ) {
				while ( curS ) {
					int    cb=curS->bord;
					//				printf("%i %i %f/ ",cb,(swrData[cb].sens)?1:0,swrData[cb].curX);
						int   oW=curW;
					if ( swrData[cb].sens ) curW++; else curW--;

					if ( curW == 0 && oW != 0) {
						line->AddBord(lastX,swrData[cb].curX,true);
					} else if ( curW != 0 && oW == 0 ) {
						lastX=swrData[cb].curX;
					}
					curS=static_cast <SweepTree*> (curS->rightElem);
				}
			}
		}
		int    curPt=curP;
		while ( curPt < nbPt && pts[curPt].x[1] <= to ) {
			int           nPt=-1;
			nPt=curPt++;

			int    cb;
			int    nbUp=0,nbDn=0;
			int    upNo=-1,dnNo=-1;
			if ( pts[nPt].dI+pts[nPt].dO == 2 ) {
				cb=pts[nPt].firstA;
				if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
					upNo=cb;
					nbUp++;
				}
				if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
					dnNo=cb;
					nbDn++;
				}
				cb=pts[nPt].lastA;
				if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
					upNo=cb;
					nbUp++;
				}
				if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
					dnNo=cb;
					nbDn++;
				}

			} else {
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
						upNo=cb;
						nbUp++;
					}
					if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
						dnNo=cb;
						nbDn++;
					}
					cb=NextAt(nPt,cb);
				}
			}

			if ( nbDn <= 0 ) {
				upNo=-1;
			}
			if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
				upNo=-1;
			}

			if ( nbUp > 1 || ( nbUp == 1 && upNo < 0 ) ) {
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != upNo ) {
							SweepTree* node=swrData[cb].misc;
							if ( node ) {
								swrData[cb].lastX=swrData[cb].curX;
								swrData[cb].lastY=swrData[cb].curY;
								swrData[cb].curX=pts[nPt].x[0];
								swrData[cb].curY=pts[nPt].x[1];
								swrData[cb].misc=NULL;
								DestroyEdge(cb,to,line,step);

								node->Remove(sTree,sEvts,true);
							}
						}
					}
					cb=NextAt(nPt,cb);
				}
			}

			// traitement du "upNo devient dnNo"
			SweepTree* insertionNode=NULL;
			if ( dnNo >= 0 ) {
				if ( upNo >= 0 ) {
					SweepTree* node=swrData[upNo].misc;
					swrData[upNo].misc=NULL;
					swrData[upNo].lastX=swrData[upNo].curX;
					swrData[upNo].lastY=swrData[upNo].curY;
					swrData[upNo].curX=pts[nPt].x[0];
					swrData[upNo].curY=pts[nPt].x[1];
					DestroyEdge(upNo,to,line,step);

					node->ConvertTo(this,dnNo,1,nPt);

					swrData[dnNo].misc=node;
					insertionNode=node;
					CreateEdge(dnNo,to,step);
				} else {
					SweepTree* node=SweepTree::AddInList(this,dnNo,1,nPt,sTree,this);
					swrData[dnNo].misc=node;
					node->Insert(sTree,sEvts,this,nPt,true);
					insertionNode=node;
					CreateEdge(dnNo,to,step);
				}
			}

			if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != dnNo ) {
							SweepTree* node=SweepTree::AddInList(this,cb,1,nPt,sTree,this);
							swrData[cb].misc=node;
							node->InsertAt(sTree,sEvts,this,insertionNode,nPt,true);
							CreateEdge(cb,to,step);
						}
					}
					cb=NextAt(nPt,cb);
				}
			}
		}
		curP=curPt;
		if ( curPt > 0 ) pos=pts[curPt-1].x[1]; else pos=to;
	}
	if ( sTree.racine ) {
		SweepTree* curS=static_cast <SweepTree*> (sTree.racine->Leftmost());
		while ( curS ) {
			int    cb=curS->bord;
			AvanceEdge(cb,to,line,exact,step);
			curS=static_cast <SweepTree*> (curS->rightElem);
		}
	}
}
void              Shape::Scan(float &pos,int &curP,float to,AlphaLigne* line,bool exact,float step)
{
	if ( pos >= to ) return;
	if ( pos < to ) {
		// pas de trapezes dans le cas de l'alphaline
/*		if ( sTree.racine ) {
			SweepTree* curS=static_cast <SweepTree*> (sTree.racine->Leftmost());
			while ( curS ) {
				int    lastGuess=-1;
				int    cb=curS->bord;
				//				printf("%i %i %f/ ",cb,(swrData[cb].sens)?1:0,swrData[cb].curX);
				if ( swrData[cb].sens == false && curS->leftElem ) {
					int  lb=(static_cast <SweepTree*> (curS->leftElem))->bord;
					line->AddBord(swrData[lb].curX,to-swrData[lb].curY,swrData[cb].curX,to-swrData[cb].curY,0.0);
				}
				curS=static_cast <SweepTree*> (curS->rightElem);
			}
			//			printf("\n");
		}*/
		int    curPt=curP;
		while ( curPt < nbPt && pts[curPt].x[1] <= to ) {
			int           nPt=-1;
			nPt=curPt++;

			int    cb;
			int    nbUp=0,nbDn=0;
			int    upNo=-1,dnNo=-1;
			if ( pts[nPt].dI+pts[nPt].dO == 2 ) {
				cb=pts[nPt].firstA;
				if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
					upNo=cb;
					nbUp++;
				}
				if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
					dnNo=cb;
					nbDn++;
				}
				cb=pts[nPt].lastA;
				if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
					upNo=cb;
					nbUp++;
				}
				if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
					dnNo=cb;
					nbDn++;
				}

			} else {
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
						upNo=cb;
						nbUp++;
					}
					if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
						dnNo=cb;
						nbDn++;
					}
					cb=NextAt(nPt,cb);
				}
			}

			if ( nbDn <= 0 ) {
				upNo=-1;
			}
			if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
				upNo=-1;
			}

			if ( nbUp > 1 || ( nbUp == 1 && upNo < 0 ) ) {
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != upNo ) {
							SweepTree* node=swrData[cb].misc;
							if ( node ) {
								swrData[cb].lastX=swrData[cb].curX;
								swrData[cb].lastY=swrData[cb].curY;
								swrData[cb].curX=pts[nPt].x[0];
								swrData[cb].curY=pts[nPt].x[1];
								swrData[cb].misc=NULL;
								DestroyEdge(cb,to,line,step);

								node->Remove(sTree,sEvts,true);
							}
						}
					}
					cb=NextAt(nPt,cb);
				}
			}

			// traitement du "upNo devient dnNo"
			SweepTree* insertionNode=NULL;
			if ( dnNo >= 0 ) {
				if ( upNo >= 0 ) {
					SweepTree* node=swrData[upNo].misc;
					swrData[upNo].misc=NULL;
					swrData[upNo].lastX=swrData[upNo].curX;
					swrData[upNo].lastY=swrData[upNo].curY;
					swrData[upNo].curX=pts[nPt].x[0];
					swrData[upNo].curY=pts[nPt].x[1];
					DestroyEdge(upNo,to,line,step);

					node->ConvertTo(this,dnNo,1,nPt);

					swrData[dnNo].misc=node;
					insertionNode=node;
					CreateEdge(dnNo,to,step);
					swrData[dnNo].guess=swrData[upNo].guess;
				} else {
					SweepTree* node=SweepTree::AddInList(this,dnNo,1,nPt,sTree,this);
					swrData[dnNo].misc=node;
					node->Insert(sTree,sEvts,this,nPt,true);
					insertionNode=node;
					CreateEdge(dnNo,to,step);
				}
			}

			if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != dnNo ) {
							SweepTree* node=SweepTree::AddInList(this,cb,1,nPt,sTree,this);
							swrData[cb].misc=node;
							node->InsertAt(sTree,sEvts,this,insertionNode,nPt,true);
							CreateEdge(cb,to,step);
						}
					}
					cb=NextAt(nPt,cb);
				}
			}
		}
		curP=curPt;
		if ( curPt > 0 ) pos=pts[curPt-1].x[1]; else pos=to;
	}
	if ( sTree.racine ) {
		SweepTree* curS=static_cast <SweepTree*> (sTree.racine->Leftmost());
		while ( curS ) {
			int    cb=curS->bord;
			AvanceEdge(cb,to,line,exact,step);
			curS=static_cast <SweepTree*> (curS->rightElem);
		}
	}
}
void              Shape::QuickScan(float &pos,int &curP,float to,FloatLigne* line,bool /*exact*/,float step)
{
	if ( pos >= to ) return;
	if ( pos < to ) {
		if ( nbQRas > 1 ) {
			int curW=0;
			float  lastX=0,lastY=0;
			int    lastGuess=-1,lastB=-1;
			for (int i=0;i<nbQRas;i++) {
				int   cb=qrsData[i].bord;
				int   oW=curW;
				if ( swrData[cb].sens ) curW++; else curW--;

				if ( curW == 0 && oW != 0) {
					lastGuess=line->AddBord(lastX,to-lastY,swrData[cb].curX,to-swrData[cb].curY,0.0,lastGuess);
					swrData[cb].guess=lastGuess;
					if ( lastB >= 0 ) swrData[lastB].guess=lastGuess-1;
				} else if ( curW != 0 && oW == 0 ) {
					lastX=swrData[cb].curX;
					lastY=swrData[cb].curY;
					lastB=cb;
					swrData[cb].guess=-1;
				} else {
					swrData[cb].guess=-1;
				}
			}
		}
		int    curPt=curP;
		while ( curPt < nbPt && pts[curPt].x[1] <= to ) {
			int           nPt=-1;
			nPt=curPt++;

			int    cb;
			int    nbUp=0,nbDn=0;
			int    upNo=-1,dnNo=-1;
			if ( pts[nPt].dI+pts[nPt].dO == 2 ) {
				cb=pts[nPt].firstA;
				if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
					upNo=cb;
					nbUp++;
				}
				if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
					dnNo=cb;
					nbDn++;
				}
				cb=pts[nPt].lastA;
				if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
					upNo=cb;
					nbUp++;
				}
				if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
					dnNo=cb;
					nbDn++;
				}

			} else {
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
						upNo=cb;
						nbUp++;
					}
					if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
						dnNo=cb;
						nbDn++;
					}
					cb=NextAt(nPt,cb);
				}
			}

			if ( nbDn <= 0 ) {
				upNo=-1;
			}
			if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
				upNo=-1;
			}

			if ( nbUp > 1 || ( nbUp == 1 && upNo < 0 ) ) {
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != upNo ) {
							int ind=qrsData[cb].ind;
							qrsData[ind].bord=qrsData[--nbQRas].bord;
							if ( nbQRas > 0 ) qrsData[qrsData[ind].bord].ind=ind;

							swrData[cb].lastX=swrData[cb].curX;
							swrData[cb].lastY=swrData[cb].curY;
							swrData[cb].curX=pts[nPt].x[0];
							swrData[cb].curY=pts[nPt].x[1];
							swrData[cb].misc=NULL;
							DestroyEdge(cb,to,line,step);
						}
					}
					cb=NextAt(nPt,cb);
				}
			}

			// traitement du "upNo devient dnNo"
			if ( dnNo >= 0 ) {
				if ( upNo >= 0 ) {
					int ind=qrsData[upNo].ind;
					qrsData[ind].bord=dnNo;
					qrsData[dnNo].ind=ind;
					swrData[upNo].lastX=swrData[upNo].curX;
					swrData[upNo].lastY=swrData[upNo].curY;
					swrData[upNo].curX=pts[nPt].x[0];
					swrData[upNo].curY=pts[nPt].x[1];
					swrData[upNo].misc=NULL;
					DestroyEdge(upNo,to,line,step);

					CreateEdge(dnNo,to,step);
					swrData[dnNo].guess=swrData[upNo].guess;
				} else {
					int ind=nbQRas++;
					qrsData[ind].bord=dnNo;
					qrsData[dnNo].ind=ind;
					CreateEdge(dnNo,to,step);
				}
			}

			if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != dnNo ) {
							int ind=nbQRas++;
							qrsData[ind].bord=cb;
							qrsData[cb].ind=ind;
							CreateEdge(cb,to,step);
						}
					}
					cb=NextAt(nPt,cb);
				}
			}
		}
		curP=curPt;
		if ( curPt > 0 ) pos=pts[curPt-1].x[1]; else pos=to;
	}
	for (int i=0;i<nbQRas;i++) {
		int cb=qrsData[i].bord;
		AvanceEdge(cb,to,line,true,step);
		qrsData[i].x=swrData[cb].curX;
	}
	if ( nbQRas > 1 ) {
		qsort(qrsData,nbQRas,sizeof(quick_raster_data),CmpQuickRaster);
		for (int i=0;i<nbQRas;i++) qrsData[qrsData[i].bord].ind=i;
	}
}
void              Shape::QuickScan(float &pos,int &curP,float to,FillRule directed,BitLigne* line,bool /*exact*/,float step)
{
	if ( pos >= to ) return;
	if ( pos < to ) {
		if ( nbQRas > 1 ) {
			int curW=0;
			float  lastX=0;
			if ( directed == fill_oddEven ) {
				for (int i=0;i<nbQRas;i++) {
					int   cb=qrsData[i].bord;
					//					int   oW=curW;
					curW++;
					curW&=0x00000001;
					if ( curW == 0 ) {
						line->AddBord(lastX,swrData[cb].curX,true);
					} else {
						lastX=swrData[cb].curX;
					}
				}
			} else if ( directed == fill_positive ) {
				// doesn't behave correctly; no way i know to do this without a ConvertToShape()
				for (int i=0;i<nbQRas;i++) {
					int   cb=qrsData[i].bord;
					int   oW=curW;
					if ( swrData[cb].sens ) curW++; else curW--;

					if ( curW <= 0 && oW > 0) {
						line->AddBord(lastX,swrData[cb].curX,true);
					} else if ( curW > 0 && oW <= 0 ) {
						lastX=swrData[cb].curX;
					}
				}
			} else if ( directed == fill_nonZero ) {
				for (int i=0;i<nbQRas;i++) {
					int   cb=qrsData[i].bord;
					int   oW=curW;
					if ( swrData[cb].sens ) curW++; else curW--;

					if ( curW == 0 && oW != 0) {
						line->AddBord(lastX,swrData[cb].curX,true);
					} else if ( curW != 0 && oW == 0 ) {
						lastX=swrData[cb].curX;
					}
				}
			}
		}
		int    curPt=curP;
		while ( curPt < nbPt && pts[curPt].x[1] <= to ) {
			int           nPt=-1;
			nPt=curPt++;

			int    cb;
			int    nbUp=0,nbDn=0;
			int    upNo=-1,dnNo=-1;
			if ( pts[nPt].dI+pts[nPt].dO == 2 ) {
				cb=pts[nPt].firstA;
				if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
					upNo=cb;
					nbUp++;
				}
				if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
					dnNo=cb;
					nbDn++;
				}
				cb=pts[nPt].lastA;
				if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
					upNo=cb;
					nbUp++;
				}
				if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
					dnNo=cb;
					nbDn++;
				}

			} else {
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
						upNo=cb;
						nbUp++;
					}
					if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
						dnNo=cb;
						nbDn++;
					}
					cb=NextAt(nPt,cb);
				}
			}

			if ( nbDn <= 0 ) {
				upNo=-1;
			}
			if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
				upNo=-1;
			}

			if ( nbUp > 1 || ( nbUp == 1 && upNo < 0 ) ) {
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != upNo ) {
							int ind=qrsData[cb].ind;
							qrsData[ind].bord=qrsData[--nbQRas].bord;
							if ( nbQRas > 0 ) qrsData[qrsData[ind].bord].ind=ind;

							swrData[cb].lastX=swrData[cb].curX;
							swrData[cb].lastY=swrData[cb].curY;
							swrData[cb].curX=pts[nPt].x[0];
							swrData[cb].curY=pts[nPt].x[1];
							swrData[cb].misc=NULL;
							DestroyEdge(cb,to,line,step);
						}
					}
					cb=NextAt(nPt,cb);
				}
			}

			// traitement du "upNo devient dnNo"
			if ( dnNo >= 0 ) {
				if ( upNo >= 0 ) {
					int ind=qrsData[upNo].ind;
					qrsData[ind].bord=dnNo;
					qrsData[dnNo].ind=ind;
					swrData[upNo].lastX=swrData[upNo].curX;
					swrData[upNo].lastY=swrData[upNo].curY;
					swrData[upNo].curX=pts[nPt].x[0];
					swrData[upNo].curY=pts[nPt].x[1];
					swrData[upNo].misc=NULL;
					DestroyEdge(upNo,to,line,step);

					CreateEdge(dnNo,to,step);
				} else {
					int ind=nbQRas++;
					qrsData[ind].bord=dnNo;
					qrsData[dnNo].ind=ind;
					CreateEdge(dnNo,to,step);
				}
			}

			if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != dnNo ) {
							int ind=nbQRas++;
							qrsData[ind].bord=cb;
							qrsData[cb].ind=ind;
							CreateEdge(cb,to,step);
						}
					}
					cb=NextAt(nPt,cb);
				}
			}
		}
		curP=curPt;
		if ( curPt > 0 ) pos=pts[curPt-1].x[1]; else pos=to;
	}
	for (int i=0;i<nbQRas;i++) {
		int cb=qrsData[i].bord;
		AvanceEdge(cb,to,line,true,step);
		qrsData[i].x=swrData[cb].curX;
	}
	if ( nbQRas > 1 ) {
		qsort(qrsData,nbQRas,sizeof(quick_raster_data),CmpQuickRaster);
		for (int i=0;i<nbQRas;i++) qrsData[qrsData[i].bord].ind=i;
	}
}

void              Shape::QuickScan(float &pos,int &curP,float to,AlphaLigne* line,bool /*exact*/,float step)
{
	if ( pos >= to ) return;
	if ( pos < to ) {
		// pas de trapezes dans le cas de l'alphaline
/*		if ( nbQRas > 1 ) {
			int curW=0;
			float  lastX=0,lastY=0;
			int    lastGuess=-1,lastB=-1;
			for (int i=0;i<nbQRas;i++) {
				int   cb=qrsData[i].bord;
				int   oW=curW;
				if ( swrData[cb].sens ) curW++; else curW--;

				if ( curW == 0 && oW != 0) {
					line->AddBord(lastX,to-lastY,swrData[cb].curX,to-swrData[cb].curY,0.0);
				} else if ( curW != 0 && oW == 0 ) {
					lastX=swrData[cb].curX;
					lastY=swrData[cb].curY;
					lastB=cb;
				}
			}
		}*/
		int    curPt=curP;
		while ( curPt < nbPt && pts[curPt].x[1] <= to ) {
			int           nPt=-1;
			nPt=curPt++;

			int    cb;
			int    nbUp=0,nbDn=0;
			int    upNo=-1,dnNo=-1;
			if ( pts[nPt].dI+pts[nPt].dO == 2 ) {
				cb=pts[nPt].firstA;
				if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
					upNo=cb;
					nbUp++;
				}
				if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
					dnNo=cb;
					nbDn++;
				}
				cb=pts[nPt].lastA;
				if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
					upNo=cb;
					nbUp++;
				}
				if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
					dnNo=cb;
					nbDn++;
				}

			} else {
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
						upNo=cb;
						nbUp++;
					}
					if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
						dnNo=cb;
						nbDn++;
					}
					cb=NextAt(nPt,cb);
				}
			}

			if ( nbDn <= 0 ) {
				upNo=-1;
			}
			if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
				upNo=-1;
			}

			if ( nbUp > 1 || ( nbUp == 1 && upNo < 0 ) ) {
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != upNo ) {
							int ind=qrsData[cb].ind;
							qrsData[ind].bord=qrsData[--nbQRas].bord;
							if ( nbQRas > 0 ) qrsData[qrsData[ind].bord].ind=ind;

							swrData[cb].lastX=swrData[cb].curX;
							swrData[cb].lastY=swrData[cb].curY;
							swrData[cb].curX=pts[nPt].x[0];
							swrData[cb].curY=pts[nPt].x[1];
							swrData[cb].misc=NULL;
							DestroyEdge(cb,to,line,step);
						}
					}
					cb=NextAt(nPt,cb);
				}
			}

			// traitement du "upNo devient dnNo"
			if ( dnNo >= 0 ) {
				if ( upNo >= 0 ) {
					int ind=qrsData[upNo].ind;
					qrsData[ind].bord=dnNo;
					qrsData[dnNo].ind=ind;
					swrData[upNo].lastX=swrData[upNo].curX;
					swrData[upNo].lastY=swrData[upNo].curY;
					swrData[upNo].curX=pts[nPt].x[0];
					swrData[upNo].curY=pts[nPt].x[1];
					swrData[upNo].misc=NULL;
					DestroyEdge(upNo,to,line,step);

					CreateEdge(dnNo,to,step);
					swrData[dnNo].guess=swrData[upNo].guess;
				} else {
					int ind=nbQRas++;
					qrsData[ind].bord=dnNo;
					qrsData[dnNo].ind=ind;
					CreateEdge(dnNo,to,step);
				}
			}

			if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
				cb=pts[nPt].firstA;
				while ( cb >= 0 && cb < nbAr ) {
					if ( ( aretes[cb].st > aretes[cb].en && nPt == aretes[cb].en ) || ( aretes[cb].st < aretes[cb].en && nPt == aretes[cb].st ) ) {
						if ( cb != dnNo ) {
							int ind=nbQRas++;
							qrsData[ind].bord=cb;
							qrsData[cb].ind=ind;
							CreateEdge(cb,to,step);
						}
					}
					cb=NextAt(nPt,cb);
				}
			}
		}
		curP=curPt;
		if ( curPt > 0 ) pos=pts[curPt-1].x[1]; else pos=to;
	}
	for (int i=0;i<nbQRas;i++) {
		int cb=qrsData[i].bord;
		AvanceEdge(cb,to,line,true,step);
		qrsData[i].x=swrData[cb].curX;
	}
	if ( nbQRas > 1 ) {
		qsort(qrsData,nbQRas,sizeof(quick_raster_data),CmpQuickRaster);
		for (int i=0;i<nbQRas;i++) qrsData[qrsData[i].bord].ind=i;
	}
}


/*
 * operations de bases pour la rasterization
 *
 */
void              Shape::CreateEdge(int no,float to,float step)
{
	int    cPt;
	NR::Point   dir;
	if ( aretes[no].st < aretes[no].en ) {
		cPt=aretes[no].st;
		swrData[no].sens=true;
		dir=aretes[no].dx;
	} else {
		cPt=aretes[no].en;
		swrData[no].sens=false;
		dir=-aretes[no].dx;
	}

	swrData[no].lastX=swrData[no].curX=pts[cPt].x[0];
	swrData[no].lastY=swrData[no].curY=pts[cPt].x[1];
	if ( fabs(dir[1]) < 0.000001 ) {
		swrData[no].dxdy=0;
	} else {
		swrData[no].dxdy=dir[0]/dir[1];
	}
	if ( fabs(dir[0]) < 0.000001 ) {
		swrData[no].dydx=0;
	} else {
		swrData[no].dydx=dir[1]/dir[0];
	}
	swrData[no].calcX=swrData[no].curX+(to-step-swrData[no].curY)*swrData[no].dxdy;
	swrData[no].guess=-1;
}
void              Shape::DestroyEdge(int /*no*/,float /*to*/,float /*step*/)
{
}
void              Shape::AvanceEdge(int no,float to,bool exact,float step)
{
	if ( exact ) {
		NR::Point  dir,stp;
		if ( swrData[no].sens ) {
			stp=pts[aretes[no].st].x;
			dir=aretes[no].dx;
		} else {
			stp=pts[aretes[no].en].x;
			dir=-aretes[no].dx;
		}
		if ( fabs(dir[1]) < 0.000001 ) {
			swrData[no].calcX=stp[0]+dir[0];
		} else {
			swrData[no].calcX=stp[0]+((to-stp[1])*dir[0])/dir[1];
		}
	} else {
		swrData[no].calcX+=step*swrData[no].dxdy;
	}
	swrData[no].lastX=swrData[no].curX;
	swrData[no].lastY=swrData[no].curY;
	swrData[no].curX=swrData[no].calcX;
	swrData[no].curY=to;
}

/*
 * specialisation par type de structure utilise
 */
void              Shape::DestroyEdge(int no,float to,FloatLigne* line,float /*step*/)
{
	if ( swrData[no].sens ) {
		if ( swrData[no].curX < swrData[no].lastX ) {
			swrData[no].guess=line->AddBordR(swrData[no].curX,to-swrData[no].curY,swrData[no].lastX,to-swrData[no].lastY,-swrData[no].dydx,swrData[no].guess);
		} else if ( swrData[no].curX > swrData[no].lastX ) {
			swrData[no].guess=line->AddBord(swrData[no].lastX,-(to-swrData[no].lastY),swrData[no].curX,-(to-swrData[no].curY),swrData[no].dydx,swrData[no].guess);
		}
	} else {
		if ( swrData[no].curX < swrData[no].lastX ) {
			swrData[no].guess=line->AddBordR(swrData[no].curX,-(to-swrData[no].curY),swrData[no].lastX,-(to-swrData[no].lastY),swrData[no].dydx,swrData[no].guess);
		} else if ( swrData[no].curX > swrData[no].lastX ) {
			swrData[no].guess=line->AddBord(swrData[no].lastX,to-swrData[no].lastY,swrData[no].curX,to-swrData[no].curY,-swrData[no].dydx,swrData[no].guess);
		}
	}
}
void              Shape::AvanceEdge(int no,float to,FloatLigne* line,bool exact,float step)
{
	AvanceEdge(no,to,exact,step);

	if ( swrData[no].sens ) {
		if ( swrData[no].curX < swrData[no].lastX ) {
			swrData[no].guess=line->AddBordR(swrData[no].curX,to-swrData[no].curY,swrData[no].lastX,to-swrData[no].lastY,-swrData[no].dydx,swrData[no].guess);
		} else if ( swrData[no].curX > swrData[no].lastX ) {
			swrData[no].guess=line->AddBord(swrData[no].lastX,-(to-swrData[no].lastY),swrData[no].curX,-(to-swrData[no].curY),swrData[no].dydx,swrData[no].guess);
		}
	} else {
		if ( swrData[no].curX < swrData[no].lastX ) {
			swrData[no].guess=line->AddBordR(swrData[no].curX,-(to-swrData[no].curY),swrData[no].lastX,-(to-swrData[no].lastY),swrData[no].dydx,swrData[no].guess);
		} else if ( swrData[no].curX > swrData[no].lastX ) {
			swrData[no].guess=line->AddBord(swrData[no].lastX,to-swrData[no].lastY,swrData[no].curX,to-swrData[no].curY,-swrData[no].dydx,swrData[no].guess);
		}
	}
}
void              Shape::DestroyEdge(int no,float /*to*/,BitLigne* line,float /*step*/)
{
	if ( swrData[no].sens ) {
		if ( swrData[no].curX < swrData[no].lastX ) {
			line->AddBord(swrData[no].curX,swrData[no].lastX,false);
		} else if ( swrData[no].curX > swrData[no].lastX ) {
			line->AddBord(swrData[no].lastX,swrData[no].curX,false);
		}
	} else {
		if ( swrData[no].curX < swrData[no].lastX ) {
			line->AddBord(swrData[no].curX,swrData[no].lastX,false);
		} else if ( swrData[no].curX > swrData[no].lastX ) {
			line->AddBord(swrData[no].lastX,swrData[no].curX,false);
		}
	}
}
void              Shape::AvanceEdge(int no,float to,BitLigne* line,bool exact,float step)
{
	AvanceEdge(no,to,exact,step);

	if ( swrData[no].sens ) {
		if ( swrData[no].curX < swrData[no].lastX ) {
			line->AddBord(swrData[no].curX,swrData[no].lastX,false);
		} else if ( swrData[no].curX > swrData[no].lastX ) {
			line->AddBord(swrData[no].lastX,swrData[no].curX,false);
		}
	} else {
		if ( swrData[no].curX < swrData[no].lastX ) {
			line->AddBord(swrData[no].curX,swrData[no].lastX,false);
		} else if ( swrData[no].curX > swrData[no].lastX ) {
			line->AddBord(swrData[no].lastX,swrData[no].curX,false);
		}
	}
}
void              Shape::DestroyEdge(int no,float /*to*/,AlphaLigne* line,float /*step*/)
{
	if ( swrData[no].sens ) {
		if ( swrData[no].curX <= swrData[no].lastX ) {
			line->AddBord(swrData[no].curX,0,swrData[no].lastX,swrData[no].curY-swrData[no].lastY,-swrData[no].dydx);
		} else if ( swrData[no].curX > swrData[no].lastX ) {
			line->AddBord(swrData[no].lastX,0,swrData[no].curX,swrData[no].curY-swrData[no].lastY,swrData[no].dydx);
		}
	} else {
		if ( swrData[no].curX <= swrData[no].lastX ) {
			line->AddBord(swrData[no].curX,0,swrData[no].lastX,swrData[no].lastY-swrData[no].curY,swrData[no].dydx);
		} else if ( swrData[no].curX > swrData[no].lastX ) {
			line->AddBord(swrData[no].lastX,0,swrData[no].curX,swrData[no].lastY-swrData[no].curY,-swrData[no].dydx);
		}
	}
}
void              Shape::AvanceEdge(int no,float to,AlphaLigne* line,bool exact,float step)
{
	AvanceEdge(no,to,exact,step);

	if ( swrData[no].sens ) {
		if ( swrData[no].curX <= swrData[no].lastX ) {
			line->AddBord(swrData[no].curX,0,swrData[no].lastX,swrData[no].curY-swrData[no].lastY,-swrData[no].dydx);
		} else if ( swrData[no].curX > swrData[no].lastX ) {
			line->AddBord(swrData[no].lastX,0,swrData[no].curX,swrData[no].curY-swrData[no].lastY,swrData[no].dydx);
		}
	} else {
		if ( swrData[no].curX <= swrData[no].lastX ) {
			line->AddBord(swrData[no].curX,0,swrData[no].lastX,swrData[no].lastY-swrData[no].curY,swrData[no].dydx);
		} else if ( swrData[no].curX > swrData[no].lastX ) {
			line->AddBord(swrData[no].lastX,0,swrData[no].curX,swrData[no].lastY-swrData[no].curY,-swrData[no].dydx);
		}
	}
}


