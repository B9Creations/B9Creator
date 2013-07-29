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
#include "worldview.h"


class B9LayoutProjectData;
class WorldView;
class ModelData;
class B9ModelInstance;
class SliceDebugger;
class B9SupportStructure;

class B9Layout : public QMainWindow
{
	Q_OBJECT

public:
    B9Layout(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~B9Layout();
    std::vector<B9ModelInstance*> GetAllInstances();
    std::vector<B9ModelInstance*> GetSelectedInstances();
    std::vector<ModelData*> GetAllModelData(){return ModelDataList;}
    B9LayoutProjectData* ProjectData(){return project;}

signals:
    void eventHiding();


public slots:

	//debug interface
	void OpenDebugWindow();

	//file
	void New();
    QString Open(bool withoutVisuals = false);
	void Save();
	void SaveAs();

	//interface
    void OnChangeTab(int idx);
    void SetXYPixelSizePreset(QString size);
	void SetZLayerThickness(QString thick);
	void SetProjectorX(QString);
	void SetProjectorY(QString);
    void SetProjectorPreset(int index);
    void SetZHeight(QString z);
    void SetAttachmentSurfaceThickness(QString num);
    void UpdateBuildSpaceUI();


    void BuildInterface();
    void UpdateInterface();//sets the translation interface fields acourding to what instance/instances are selected.
	void PushTranslations();
    void OnModelSpinSliderChanged(int val);//when the spin slider changes value by the user.
    void OnModelSpinSliderReleased();
	void LockScale(bool lock);

    //TOOLS
    void SetTool(QString toolname);//calls the functions below

    //ModelTools interface
    void SetToolModelSelect();
    void SetToolModelMove();
    void SetToolModelSpin();
    void SetToolModelOrientate();
    void SetToolModelScale();

    //SupportTools interface
    void SetToolSupportModify();
    void SetToolSupportAdd();
    void SetToolSupportDelete();


    void ExitToolAction();//use to panic out of a mouse down tool action.
	
	//model
    B9ModelInstance* AddModel(QString filepath = "", bool bypassVisuals = false);
	void RemoveAllInstances();
	void CleanModelData();//cleans andy modeldata that does not have a instance!
	void AddTagToModelList(QListWidgetItem* item);

    B9ModelInstance* FindInstance(QListWidgetItem* item);//given a item you can find the connected instance
	
	//selection
	void RefreshSelectionsFromList();//searches through all active listitems and selects their corresponding instance;
    void Select(B9ModelInstance* inst);//selects the given instance
    void DeSelect(B9ModelInstance* inst);//de-selects the given instance
    void SelectOnly(B9ModelInstance* inst);//deselects eveything and selects only the instance
	void DeSelectAll();//de-selects all instances
	void SetSelectionPos(double x, double y, double z, int axis = 0);
    void SetSelectionRot(QVector3D newRot);
    void SetSelectionScale(double x, double y, double z, int axis = 0);
    void SetSelectionFlipped(bool flipped);
	void DropSelectionToFloor();
    void ResetSelectionRotation();
    void DuplicateSelection();
    void DeleteSelection();//delete whatever is selected - support or instance..
	void DeleteSelectedInstances();

    //Support Mode
    void SetSupportMode(bool tog);//Sets everything up for editing supports for the selected instance.
                            //when either the tab is clicked or the menu item - a selected instance
                            //can be assumed
    void FillSupportList();
    B9SupportStructure* FindSupportByName(QString name);
    void RefreshSupportSelectionsFromList();//Called when the user selects stuff in the support list

    void SelectSupport(B9SupportStructure* sup);
    std::vector<B9SupportStructure*>* GetSelectedSupports();
    bool IsSupportSelected(B9SupportStructure* sup);
    void DeSelectSupport(B9SupportStructure* sup);
    void DeSelectAllSupports();
    void DeleteSelectedSupports();//called from remove button.
    void DeleteSupport(B9SupportStructure* pSup);
    void MakeSelectedSupportsVertical();

    //Support Interface
    void OnSupportPropertiesChanged();//triggered when anything changes in the support properties.
    void OnBasePlatePropertiesChanged();
    void PushSupportProperties();//fills the support properties with relevent data.
    void PushBasePlateProperties();
    void WriteSupportPropertiesToRegistry(B9SupportStructure *sup);
    void ResetSupportDefaults();//connected to push button

    //returns a valid instance if we are editing it in support mode.
    B9ModelInstance* SupportModeInst();

	//slicing
    bool SliceWorld();//prompts the user to slice to world to different formats.
    bool SliceWorldToJob(QString filename);//slices to whole world to a job file
    bool SliceWorldToSlc(QString filename);//slices to whole world to a slc file
	void CancelSlicing(); //connected to the progress bar to stop slicing.

    //exporting
    void PromptExportToSTL();//export the whole layout to a stl file.
        bool ExportToSTL(QString filename);



    //events
	void keyPressEvent(QKeyEvent * event );
	void keyReleaseEvent(QKeyEvent * event );
    void mouseReleaseEvent(QMouseEvent * event);
    void mousePressEvent(QMouseEvent * event);

private:
    Ui::B9Layout ui;
	WorldView* pWorldView;
	SliceDebugger* pslicedebugger;
    B9LayoutProjectData* project;


    std::vector<ModelData*> ModelDataList;

    bool cancelslicing;

    //support mode
    B9ModelInstance* currInstanceInSupportMode;
    QVector3D oldPan;
    QVector3D oldRot;
    float oldZoom;
    QString oldTool;
    std::vector<B9SupportStructure*> currSelectedSupports;//what supports are currently in selection.



    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent * event );
    void contextMenuEvent(QContextMenuEvent *event);
};

#endif // B93D_H
