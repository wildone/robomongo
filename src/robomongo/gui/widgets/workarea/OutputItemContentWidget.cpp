#include "robomongo/gui/widgets/workarea/OutputItemContentWidget.h"

#include <QVBoxLayout>
#include <Qsci/qscilexerjavascript.h>

#include "robomongo/core/AppRegistry.h"
#include "robomongo/core/settings/SettingsManager.h"
#include "robomongo/core/utils/QtUtils.h"
#include "robomongo/gui/widgets/workarea/JsonPrepareThread.h"
#include "robomongo/gui/widgets/workarea/BsonTreeWidget.h"
#include "robomongo/gui/widgets/workarea/BsonTableView.h"
#include "robomongo/gui/widgets/workarea/BsonTableModel.h"
#include "robomongo/gui/editors/PlainJavaScriptEditor.h"
#include "robomongo/gui/widgets/workarea/CollectionStatsTreeWidget.h"
#include "robomongo/gui/GuiRegistry.h"
#include "robomongo/gui/editors/JSLexer.h"
#include "robomongo/gui/editors/FindFrame.h"

namespace Robomongo
{
    OutputItemContentWidget::OutputItemContentWidget(MongoShell *shell, const QString &text) :
        _bsonTable(NULL),
        _isTextModeSupported(true),
        _isTreeModeSupported(false),
        _isTableModeSupported(false),
        _isCustomModeSupported(false),
        _text(text),
        _sourceIsText(true),
        _shell(shell)
    {
        setup();
    }

    OutputItemContentWidget::OutputItemContentWidget(MongoShell *shell, const std::vector<MongoDocumentPtr> &documents, const MongoQueryInfo &queryInfo) :
        _bsonTable(NULL),
        _isTextModeSupported(true),
        _isTreeModeSupported(true),
        _isTableModeSupported(true),
        _isCustomModeSupported(false),
        _documents(documents),
        _queryInfo(queryInfo),
        _sourceIsText(false),
        _shell(shell)
    {
        setup();
    }

    OutputItemContentWidget::OutputItemContentWidget(MongoShell *shell, const QString &type, const std::vector<MongoDocumentPtr> &documents, const MongoQueryInfo &queryInfo) :
        _bsonTable(NULL),
        _isTextModeSupported(true),
        _isTreeModeSupported(true),
        _isTableModeSupported(true),
        _isCustomModeSupported(true),
        _documents(documents),
        _queryInfo(queryInfo),
        _sourceIsText(false),
        _type(type),
        _shell(shell)
    {
        setup();
    }


    OutputItemContentWidget::~OutputItemContentWidget()
    {
    /*    if (_thread)
            _thread->exit = true;*/
    }

    void OutputItemContentWidget::update(const QString &text)
    {
        _text = text;
        _sourceIsText = true;
        _isFirstPartRendered = false;
        markUninitialized();

        if (_bson) {
            _stack->removeWidget(_bson);
            delete _bson;
            _bson = NULL;
        }

        if (_log) {
            _stack->removeWidget(_log);
            delete _log;
            _log = NULL;
        }
    }

    void OutputItemContentWidget::update(const std::vector<MongoDocumentPtr> &documents)
    {
        _documents = documents;
        _sourceIsText = false;
        _isFirstPartRendered = false;
        markUninitialized();

        if (_bson) {
            _stack->removeWidget(_bson);
            delete _bson;
            _bson = NULL;
        }

        if (_log) {
            _stack->removeWidget(_log);
            delete _log;
            _log = NULL;
        }
    }

    void OutputItemContentWidget::setup()
    {
        markUninitialized();

        _isFirstPartRendered = false;
        _log = NULL;
        _bson = NULL;
        _thread = NULL;

        setContentsMargins(0, 0, 0, 0);
        _stack = new QStackedWidget;

        QVBoxLayout *layout = new QVBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(_stack);
        setLayout(layout);
    }

    void OutputItemContentWidget::showText()
    {
        if (_isTextModeSupported)
        {
            if (!_isTextModeInitialized)
            {
                _log = configureLogText();
                if (_sourceIsText)
                {
                    _log->sciScintilla()->setText(_text);
                }
                else
                {
                    if (_documents.size() > 0)
                    {
                        _log->sciScintilla()->setText("Loading...");
                        _thread = new JsonPrepareThread(_documents, AppRegistry::instance().settingsManager()->uuidEncoding(), AppRegistry::instance().settingsManager()->timeZone());
                        VERIFY(connect(_thread, SIGNAL(done()), this, SLOT(jsonPrepared())));
                        VERIFY(connect(_thread, SIGNAL(partReady(QString)), this, SLOT(jsonPartReady(QString))));
                        VERIFY(connect(_thread, SIGNAL(finished()), _thread, SLOT(deleteLater())));
                        _thread->start();
                    }
                }
                _stack->addWidget(_log);
                _isTextModeInitialized = true;
            }
            _stack->setCurrentWidget(_log);
        }
    }

    void OutputItemContentWidget::showTree()
    {
        if (!_isTreeModeSupported) {
            // try to downgrade to text mode
            showText();
            return;
        }

        if (!_isTreeModeInitialized) {
            _bson = new BsonTreeWidget(_shell);
            _bson->setDocuments(_documents, _queryInfo);
            _stack->addWidget(_bson);
            _isTreeModeInitialized = true;
        }

        _stack->setCurrentWidget(_bson);
    }

    void OutputItemContentWidget::showCustom()
    {
        if (!_isCustomModeSupported) {
            // try to downgrade to tree mode
            showTree();
            return;
        }

        QWidget *customWidget = NULL;

        if (!_isCustomModeInitialized) {

            if (_type == "collectionStats") {
                _collectionStats = new CollectionStatsTreeWidget(_shell);
                _collectionStats->setDocuments(_documents);
                customWidget = _collectionStats;
            }

            if (customWidget)
                _stack->addWidget(_collectionStats);
            _isCustomModeInitialized = true;
        }

        if (_collectionStats)
            _stack->setCurrentWidget(_collectionStats);
    }

    void OutputItemContentWidget::showTable()
    {
        if (!_isTableModeSupported) {
            // try to downgrade to text mode
            showText();
            return;
        }

        if (!_isTableModeInitialized) {
            _bsonTable = new BsonTableView(_shell,_queryInfo);
            BsonTableModel *mod = new BsonTableModel(_documents,_bsonTable);
            _bsonTable->setModel(mod);
            _stack->addWidget(_bsonTable);
            _isTableModeInitialized = true;
        }

        _stack->setCurrentWidget(_bsonTable);
    }

    void OutputItemContentWidget::markUninitialized()
    {
        _isTextModeInitialized = false;
        _isTreeModeInitialized = false;
        _isCustomModeInitialized = false;
        _isTableModeInitialized = false;
    }

    void OutputItemContentWidget::jsonPrepared()
    {
        // seems that it is wrong to call any method on thread,
        // because thread already can be disposed.
        // QThread *thread = static_cast<QThread *>(sender());
        // thread->quit();
    }

    void OutputItemContentWidget::jsonPartReady(const QString &json)
    {
        // check that this is our current thread
        JsonPrepareThread *thread = qobject_cast<JsonPrepareThread *>(sender());
        if (thread && thread != _thread)
        {
            // close previous thread
            thread->stop();
            thread->wait();
        }
        else
        {
            if (_log)
            {
                _log->setUpdatesEnabled(false);
                if (_isFirstPartRendered)
                    _log->sciScintilla()->append(json);
                else
                    _log->sciScintilla()->setText(json);
                _log->setUpdatesEnabled(true);
                _isFirstPartRendered = true;
            }
        }
    }

    FindFrame *Robomongo::OutputItemContentWidget::configureLogText()
    {
        const QFont &textFont = GuiRegistry::instance().font();

        QsciLexerJavaScript *javaScriptLexer = new JSLexer(this);
        javaScriptLexer->setFont(textFont);

        FindFrame *_logText = new FindFrame(this);
        _logText->sciScintilla()->setLexer(javaScriptLexer);
        _logText->sciScintilla()->setTabWidth(4);        
        _logText->sciScintilla()->setBraceMatching(QsciScintilla::StrictBraceMatch);
        _logText->sciScintilla()->setFont(textFont);
        _logText->sciScintilla()->setReadOnly(true);
        _logText->sciScintilla()->setWrapMode((QsciScintilla::WrapMode) QsciScintilla::SC_WRAP_NONE);
        _logText->sciScintilla()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        _logText->sciScintilla()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        // Wrap mode turned off because it introduces huge performance problems
        // even for medium size documents.    
        _logText->sciScintilla()->setStyleSheet("QFrame {background-color: rgb(73, 76, 78); border: 1px solid #c7c5c4; border-radius: 0px; margin: 0px; padding: 0px;}");
        return _logText;
    }
}
