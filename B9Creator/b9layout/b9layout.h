/*************************************************************************************
//
//  LICENSE INFORMATION
//
//  BCreator(tm)
//  Software for the control of the 3D Printer, "B9Creator"(tm)
//
//  Copyright 2011-2012 B9Creations, LLC
//  B9Creations(tm) and B9Creator(tm) are trademarks of B9Creations, LLC
//
//  This file is part of B9Creator
//
//    B9Creator is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    B9Creator is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with B9Creator .  If not, see <http://www.gnu.org/licenses/>.
//
//  The above copyright notice and this permission notice shall be
//    included in all copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
//    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
*************************************************************************************/

#ifndef B93D_H
#define B93D_H

#include <QtGui>
#include "ui_b93dmain.h"
#include "projectdata.h"
#include "worldview.h"
#include "modeldata.h"
#include "modelinstance.h"


class WorldView;
class ProjectData;
class ModelData;
class ModelInstance;
class SliceDebugger;
class B9Layout : public QMainWindow
{
	Q_OBJECT

public:
    B9Layout(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~B9Layout();

	ProjectData* project;
	std::vector<ModelData*> ModelDataList;

signals:
    void eventHiding();


public slots:

	//debug interface
	void OpenDebugWindow();


	//file
	void New();
    QString Open();
	void Save();
	void SaveAs();


	//interface
    void SetXYPixelSizePreset(QString size);
	void SetZLayerThickness(QString thick);
	void SetProjectorX(QString);
	void SetProjectorY(QString);
    void SetProjectorPreset(int index);
    void SetZHeight(QString z);
    void UpdateBuildSpaceUI();


	//modeltranslation interface;
	void UpdateTranslationInterface();//sets the translation interface fields acourding to what instance/instances are selected.
	void PushTranslations();
	void LockScale(bool lock);


	//tools interface
	void SetToolPointer();
	void SetToolMove();
	void SetToolRotate();
    void SetToolScale();
	
	//model
	ModelInstance* AddModel(QString filepath = "");
	void RemoveAllInstances();
	void CleanModelData();//cleans andy modeldata that does not have a instance!
	void AddTagToModelList(QListWidgetItem* item);
	ModelInstance* FindInstance(QListWidgetItem* item);//given a item you can find the connected instance
	
	//selection
	void RefreshSelectionsFromList();//searches through all active listitems and selects their corresponding instance;
	void Select(ModelInstance* inst);//selects the given instance
	void DeSelect(ModelInstance* inst);//de-selects the given instance
	void SelectOnly(ModelInstance* inst);//deselects eveything and selects only the instance
	void DeSelectAll();//de-selects all instances
	void SetSelectionPos(double x, double y, double z, int axis = 0);
	void SetSelectionRot(double x, double y, double z, int axis = 0);
	void SetSelectionScale(double x, double y, double z, int axis = 0);
    void SetSelectionFlipped(int flipped);
	void DropSelectionToFloor();
	void DuplicateSelection();
	std::vector<ModelInstance*> GetSelectedInstances();
	void DeleteSelectedInstances();


	//slicing
	void SliceWorld();//prompts the user to slice to world to different formats.
	void SliceWorldToJob(QString filename);//slices to whole world to a job file
	void SliceWorldToSlc(QString filename);//slices to whole world to a slc file
	void CancelSlicing(); //connected to the progress bar to stop slicing.



	//events
	void keyPressEvent(QKeyEvent * event );
	void keyReleaseEvent(QKeyEvent * event );
	
private:
    Ui::B9Layout ui;
	WorldView* pWorldView;
	SliceDebugger* pslicedebugger;

	bool cancelslicing;


    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent * event );
    void contextMenuEvent(QContextMenuEvent *event);
};

#endif // B93D_H
