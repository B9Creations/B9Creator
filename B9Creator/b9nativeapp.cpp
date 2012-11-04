#include <QWidget>
#include "b9nativeapp.h"

// It's safe to call this function on any platform.
// It will only have an effect on the Mac.
void B9NativeApp::set_smaller_text_osx(QWidget *w)
{
    if(w==0)return;

    // By default, none of these size attributes are set.
    // If any has been set explicitly, we'll leave the widget alone.
    if (!w->testAttribute(Qt::WA_MacMiniSize) &&
        !w->testAttribute(Qt::WA_MacSmallSize) &&
        !w->testAttribute(Qt::WA_MacNormalSize) &&
        !w->testAttribute(Qt::WA_MacVariableSize))
    {
        // Is the widget is one of a number of types whose default
        // text size is too large?
        if (w->inherits("QLabel") ||
            w->inherits("QLineEdit") ||
            w->inherits("QTextEdit") ||
            w->inherits("QComboBox") ||
            w->inherits("QCheckBox") ||
            w->inherits("QRadioButton") ||
            w->inherits("QAbstractItemView"))
            // Others could be added here...
        {
            // make the text the 'normal' size
            w->setAttribute(Qt::WA_MacMiniSize);
        }
        else if( w->inherits("QPushButton") )
            // Others could be added here...
        {
            // make the text the 'normal' size
            w->setAttribute(Qt::WA_MacMiniSize);
        }
    }
}
