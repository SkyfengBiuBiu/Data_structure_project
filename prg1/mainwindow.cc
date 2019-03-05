// The main program (provided by the course), TIE-20100/TIE-20106
//
// DO ****NOT**** EDIT THIS FILE!
// (Preferably do not edit this even temporarily. And if you still decide to do so
//  (for debugging, for example), DO NOT commit any changes to git, but revert all
//  changes later. Otherwise you won't be able to get any updates/fixes to this
//  file from git!)

#include "mainprogram.hh"

// Qt generated main window code

#ifdef GRAPHICAL_GUI
#include <QCoreApplication>
#include <QFileDialog>
#include <QDir>
#include <QFont>
#include <QGraphicsItem>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QGraphicsItem>
#include <QVariant>

#include <string>
using std::string;

#include <sstream>
using std::ostringstream;

#include <cassert>

#include "mainwindow.hh"
#include "ui_mainwindow.h"

struct XroadInfo
{
    BeaconID id;
    Coord coord;
};

// Needed to be able to store BeaconID in QVariant (in QGraphicsItem)
//Q_DECLARE_METATYPE(BeaconID)
Q_DECLARE_METATYPE(XroadInfo)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mainprg_.setui(this);

    // Execute line
    connect(ui->execute_button, &QPushButton::pressed, this, &MainWindow::execute_line);

    // Line edit
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &MainWindow::execute_line);

    // File selection
    connect(ui->file_button, &QPushButton::pressed, this, &MainWindow::select_file);

    // Command selection
    // !!!!! Sort commands in alphabetical order (should not be done here, but is)
    std::sort(mainprg_.cmds_.begin(), mainprg_.cmds_.end(), [](auto& l, auto& r){ return l.cmd < r.cmd; });
    for (auto& cmd : mainprg_.cmds_)
    {
        ui->cmd_select->addItem(QString::fromStdString(cmd.cmd));
    }
    connect(ui->cmd_select, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, &MainWindow::cmd_selected);

    // Number selection
    for (auto i = 0; i <= 20; ++i)
    {
        ui->number_select->addItem(QString("%1").arg(i));
    }
    connect(ui->number_select, static_cast<void(QComboBox::*)(QString const&)>(&QComboBox::activated), this, &MainWindow::number_selected);

    // Output box
    QFont monofont("Monospace");
    monofont.setStyleHint(QFont::Monospace);
    ui->output->setFont(monofont);

    // Initialize graphics scene & view
    gscene_ = new QGraphicsScene(this);
    ui->graphics_view->setScene(gscene_);
//    for (auto x=0u; x<10; ++x)
//    {
//        for (auto y=0u; y<10; ++y)
//        {
//            QGraphicsItem* p = gscene_->addRect(10*x, 10*y, 10, 10, QPen(Qt::NoPen), QBrush(Qt::red));
//            p->setFlag(QGraphicsItem::ItemIsSelectable);
//            gscene_->addLine(10*x, 10*y, 10*(x+1), 10*y);
//            gscene_->addLine(10*x, 10*y, 10*x, 10*(y+1));
//            if (x == 9) { gscene_->addLine(10*(x+1), 10*y, 10*(x+1), 10*(y+1)); }
//            if (y == 9) { gscene_->addLine(10*x, 10*(y+1), 10*(x+1), 10*(y+1)); }
//        }
//    }
    ui->graphics_view->resetTransform();
    ui->graphics_view->setBackgroundBrush(Qt::black);

    // Selecting graphics items by mouse
    connect(gscene_, &QGraphicsScene::selectionChanged, this, &MainWindow::scene_selection_change);
//    connect(this, &MainProgram::signal_clear_selection, this, &MainProgram::clear_selection);

    // Zoom slider changes graphics view scale
    connect(ui->zoom_plus, &QToolButton::clicked, [this]{ this->ui->graphics_view->scale(1.1, 1.1); });
    connect(ui->zoom_minus, &QToolButton::clicked, [this]{ this->ui->graphics_view->scale(1/1.1, 1/1.1); });
    connect(ui->zoom_1, &QToolButton::clicked, [this]{ this->ui->graphics_view->resetTransform(); });
    connect(ui->zoom_fit, &QToolButton::clicked, this, &MainWindow::fit_view);

    // Changing checkboxes updates view
    connect(ui->lightbeams_checkbox, &QCheckBox::clicked, this, &MainWindow::update_view);
    connect(ui->beamcolor_checkbox, &QCheckBox::clicked, this, &MainWindow::update_view);
    connect(ui->beamroutes_checkbox, &QCheckBox::clicked, this, &MainWindow::update_view);
    connect(ui->roads_checkbox, &QCheckBox::clicked, this, &MainWindow::update_view);
    connect(ui->beacons_checkbox, &QCheckBox::clicked, this, &MainWindow::update_view);
    connect(ui->beaconnames_checkbox, &QCheckBox::clicked, this, &MainWindow::update_view);

    // Unchecking lightbeams checkbox disables beam color and routes checkboxes
    connect(ui->lightbeams_checkbox, &QCheckBox::clicked,
            [this]{ this->ui->beamcolor_checkbox->setEnabled(this->ui->lightbeams_checkbox->isChecked()); });
    connect(ui->lightbeams_checkbox, &QCheckBox::clicked,
            [this]{ this->ui->beamroutes_checkbox->setEnabled(this->ui->lightbeams_checkbox->isChecked()); });

    // Unchecking beacons checkbox disables beacon names checkbox
    connect(ui->beacons_checkbox, &QCheckBox::clicked,
            [this]{ this->ui->beaconnames_checkbox->setEnabled(this->ui->beacons_checkbox->isChecked()); });

    // Changing font scale updates view
    connect(ui->fontscale, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &MainWindow::update_view);

    // Clear input button
    connect(ui->clear_input_button, &QPushButton::clicked, this, &MainWindow::clear_input_line);

    // Stop button
    connect(ui->stop_button, &QPushButton::clicked, [this](){ this->stop_pressed_ = true; });

    clear_input_line();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update_view()
{
//    ui->output->appendPlainText("Update view:");

    gscene_->clear();
    auto fontscale = ui->fontscale->value();

    auto beacons = mainprg_.ds_.all_beacons();
    for (auto& beaconid : beacons)
    {
        auto name = mainprg_.ds_.get_name(beaconid);
        auto rgb = mainprg_.ds_.get_color(beaconid);
        QColor beaconcolor(rgb.r, rgb.g, rgb.b);
        QColor namecolor = Qt::cyan;
        QColor beaconborder = Qt::white;

        auto xy = mainprg_.ds_.get_coordinates(beaconid);
        auto [x,y] = xy;

        if (xy == NO_COORD)
        {
            x = 0; y = 0;
            beaconcolor = Qt::magenta;
            namecolor = Qt::magenta;
        }

        auto groupitem = gscene_->createItemGroup({});
        groupitem->setPos(20*x, -20*y);
        groupitem->setFlag(QGraphicsItem::ItemIsSelectable);
        groupitem->setData(0, QVariant::fromValue(XroadInfo{beaconid, xy}));

        string prefix;
        if (mainprg_.prev_result.first == MainProgram::ResultType::IDLIST ||
            mainprg_.prev_result.first == MainProgram::ResultType::HIERARCHY)
        {
            auto& prev_result = std::get<MainProgram::CmdResultIDs>(mainprg_.prev_result.second);
            auto res_place = std::find(prev_result.begin(), prev_result.end(), beaconid);

            if (res_place != prev_result.end())
            {
                auto res_index = res_place - prev_result.begin();
                if (prev_result.size() > 1) { prefix = MainProgram::convert_to_string(res_index+1)+". "; }
                namecolor = Qt::red;
                beaconborder = Qt::red;
            }
        }
        else if (mainprg_.prev_result.first == MainProgram::ResultType::COORDLIST)
        {
            auto& prev_result = std::get<MainProgram::CmdResultCoords>(mainprg_.prev_result.second);
            auto res_index = prev_result.size(); // Default: not in result
            auto res_place = std::find(prev_result.begin(), prev_result.end(), xy);
            res_index = res_place - prev_result.begin();

            if (res_index < prev_result.size())
            {
                beaconborder = Qt::red;
            }
        }
        else if (mainprg_.prev_result.first == MainProgram::ResultType::FIBRELIST)
        {
            auto& prev_result = std::get<MainProgram::CmdResultFibres>(mainprg_.prev_result.second);
            auto res_index = prev_result.size(); // Default: not in result
            auto res_place = std::find_if(prev_result.begin(), prev_result.end(), [xy](auto fibre){ return fibre.first == xy; });
            res_index = res_place - prev_result.begin();

            if (res_index < prev_result.size())
            {
                beaconborder = Qt::red;
            }
        }

        if (ui->beacons_checkbox->isChecked())
        {
            QPen beaconpen(beaconborder);
            beaconpen.setWidth(0); // Cosmetic pen
            auto dotitem = gscene_->addEllipse(20*x-8*fontscale, -20*y-8*fontscale, 16*fontscale, 16*fontscale,
                                               beaconpen, QBrush(beaconcolor));
            dotitem->setZValue(1);
            groupitem->addToGroup(dotitem);
            //        dotitem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
            //        dotitem->setData(0, QVariant::fromValue(town));

            // Draw beacon names
            string label = prefix;
            if (ui->beaconnames_checkbox->isChecked())
            {
                label += name;
            }

            if (!label.empty())
            {
                auto textitem = gscene_->addSimpleText(QString::fromStdString(label));
                textitem->setZValue(1);
                textitem->setPos(20*x-8*fontscale, -20*y-27*fontscale);
                textitem->setBrush(QBrush(namecolor));
                auto font = textitem->font();
                font.setPointSizeF(font.pointSizeF()*fontscale);
                textitem->setFont(font);
        //        textitem->setTransformOriginPoint(20*town->x, -20*town->y);
                groupitem->addToGroup(textitem);
        //        textitem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
            }
        }

        // Draw light beams
        if (ui->lightbeams_checkbox->isChecked())
        {
            auto sources = mainprg_.ds_.get_lightsources(beaconid);
            for (auto& sourceid : sources)
            {
                auto vxy = mainprg_.ds_.get_coordinates(sourceid);
                auto [vx,vy] = vxy;
                QColor beamcolor = Qt::lightGray;
                Qt::PenStyle penstyle = Qt::SolidLine;

                if (vxy == NO_COORD)
                {
                    vx = 0; vy = 0;
                    beamcolor = Qt::red;
                    penstyle = Qt::DotLine;
                }

                if (ui->beamcolor_checkbox->isChecked())
                {
                    auto totcol = mainprg_.ds_.total_color(sourceid);
                    if (totcol != NO_COLOR)
                    {
                        beamcolor = QColor(totcol.r, totcol.g, totcol.b);
                    }
                }
                int zvalue = -1;
                auto linecolor = beamcolor;
                auto arrowcolor = beamcolor;

                if (mainprg_.prev_result.first == MainProgram::ResultType::HIERARCHY)
                {
                    auto& prev_result = std::get<MainProgram::CmdResultIDs>(mainprg_.prev_result.second);
                    auto res_index = prev_result.size(); // Default: not in result
                    auto res_place = std::find(prev_result.begin(), prev_result.end(), beaconid);
                    res_index = res_place - prev_result.begin();

                    if (res_index < prev_result.size())
                    {
                        if ((res_index > 0 && prev_result[res_index-1] == sourceid) ||
                            (res_index+1 < prev_result.size() && prev_result[res_index+1] == sourceid))
                        {
                            penstyle = Qt::DotLine;
                            linecolor = Qt::red;
                            zvalue = 10;
                        }
                    }
                }

    //            auto pen = QPen(linecolor);
    //            pen.setWidth(0); // "Cosmetic" pen
    //            auto lineitem = gscene_->addLine(20*x+4, -20*y+4, 20*vx+4, -20*vy+4, pen);
                auto pen = QPen(linecolor, 3*fontscale);
                auto arrowpen = QPen(arrowcolor);
                pen.setStyle(penstyle);
//                pen.setWidth(5*fontscale);

                if (ui->beamroutes_checkbox->isChecked())
                {
                    auto route = std::vector<std::pair<Coord, Cost>>{};/*mainprg_.ds_.route_any({vx, vy}, {x, y})*/;
                    if (!(route.empty() || route.front().first == NO_COORD))
                    {
                        route.pop_back(); // Remove the last segment
                        for (auto& fibre : route)
                        {
                            auto [x2, y2] = fibre.first;
                            QLineF line(QPointF(20*x2, -20*y2), QPointF(20*vx, -20*vy));
                            auto lineitem = gscene_->addLine(line, pen);
                            lineitem->setZValue(zvalue);
                            vx = x2;
                            vy = y2;
                        }
                    }
                }
                QLineF line(QPointF(20*x, -20*y), QPointF(20*vx, -20*vy));
                auto lineitem = gscene_->addLine(line, pen);
                lineitem->setZValue(zvalue);

                double const PI  = 3.141592653589793238463;
                auto arrowSize = 15*fontscale;

                double angle = std::atan2(-line.dy(), line.dx());

                QPointF arrowP1 = line.p1() + QPointF(sin(angle + PI / 3) * arrowSize,
                                                cos(angle + PI / 3) * arrowSize);
                QPointF arrowP2 = line.p1() + QPointF(sin(angle + PI - PI / 3) * arrowSize,
                                                cos(angle + PI - PI / 3) * arrowSize);

                QPolygonF arrowHead;
                arrowHead << line.p1() << arrowP1 << arrowP2;
                auto headitem = gscene_->addPolygon(arrowHead, arrowpen, QBrush(arrowpen.color()));
                headitem->setZValue(zvalue);
            }
        }
    }

    // Draw roads
    if (ui->roads_checkbox->isChecked())
    {
        auto xpoints = std::vector<Coord>{}; /*mainprg_.ds_.all_xpoints();*/
        for (auto& xy1 : xpoints)
        {
            auto [x,y] = xy1;

            QColor xroadcolor = Qt::lightGray;
            if (mainprg_.prev_result.first == MainProgram::ResultType::COORDLIST)
            {
                auto& prev_result = std::get<MainProgram::CmdResultCoords>(mainprg_.prev_result.second);
                auto res_index = prev_result.size(); // Default: not in result
                auto res_place = std::find(prev_result.begin(), prev_result.end(), xy1);
                res_index = res_place - prev_result.begin();

                if (res_index < prev_result.size())
                {
                    xroadcolor = Qt::red;
                }
            }
            else if (mainprg_.prev_result.first == MainProgram::ResultType::FIBRELIST)
            {
                auto& prev_result = std::get<MainProgram::CmdResultFibres>(mainprg_.prev_result.second);
                auto res_index = prev_result.size(); // Default: not in result
                auto res_place = std::find_if(prev_result.begin(), prev_result.end(), [xy1](auto fibre){ return fibre.first == xy1; });
                res_index = res_place - prev_result.begin();

                if (res_index < prev_result.size())
                {
                    xroadcolor = Qt::red;
                }
            }

            QPen xroadpen(xroadcolor);
            xroadpen.setWidth(0); // Cosmetic pen
            auto dotitem = gscene_->addEllipse(20*x-4*fontscale, -20*y-4*fontscale, 8*fontscale, 8*fontscale,
                                               xroadpen, QBrush(Qt::black));
            dotitem->setZValue(-1);
            dotitem->setFlag(QGraphicsItem::ItemIsSelectable);
            dotitem->setData(0, QVariant::fromValue(XroadInfo{NO_ID, xy1}));

            auto fibres = std::vector<std::pair<Coord, Cost>>{}; /*mainprg_.ds_.get_fibres_from(xy1);*/
            for (auto& fibre : fibres)
            {
                auto xy2 = fibre.first;
                auto [rx,ry] = xy2;

                QColor linecolor = Qt::white;
                int zvalue = -2;

                if (mainprg_.prev_result.first == MainProgram::ResultType::PATH)
                {
                    auto& prev_result = std::get<MainProgram::CmdResultFibres>(mainprg_.prev_result.second);
                    auto res_index = prev_result.size(); // Default: not in result
                    auto res_place = std::find_if(prev_result.begin(), prev_result.end(), [xy1](auto fibre){ return fibre.first == xy1; });
                    res_index = res_place - prev_result.begin();

                    if (res_index < prev_result.size())
                    {
                        if ((res_index > 0 && prev_result[res_index-1].first == xy2) ||
                            (res_index+1 < prev_result.size() && prev_result[res_index+1].first == xy2))
                        {
                            linecolor = Qt::red;
                            zvalue = 10;
                        }
                    }
                }

                if (mainprg_.prev_result.first == MainProgram::ResultType::CYCLE)
                {
                    auto& prev_result = std::get<MainProgram::CmdResultCoords>(mainprg_.prev_result.second);
                    auto res_index = prev_result.size(); // Default: not in result
                    auto res_place = std::find(prev_result.begin(), prev_result.end(), xy1);
                    res_index = res_place - prev_result.begin();

                    if (res_index < prev_result.size())
                    {
                        if ((res_index > 0 && prev_result[res_index-1] == xy2) ||
                            (res_index+1 < prev_result.size() && prev_result[res_index+1] == xy2))
                        {
                            linecolor = Qt::red;
                            zvalue = 10;
                        }
                    }
                }

                auto pen = QPen(linecolor);
                pen.setWidth(0); // "Cosmetic" pen
                auto lineitem = gscene_->addLine(20*x, -20*y, 20*rx, -20*ry, pen);
                lineitem->setZValue(zvalue);
    //            auto pen = QPen(linecolor);
    //            pen.setWidth(0); // "Cosmetic" pen
    //            auto arrow = new Arrow(QPointF(20*x+4, -20*y+4), QPointF(20*rx+4, -20*ry+4));
    //            arrow->setPen(pen);
    //            gscene_->addItem(arrow);
    //            arrow->setZValue(-1);
            }
        }
    }
}

void MainWindow::output_text(ostringstream& output)
{
    string outstr = output.str();
    if (!outstr.empty())
    {
        if (outstr.back() == '\n') { outstr.pop_back(); } // Remove trailing newline
        ui->output->appendPlainText(QString::fromStdString(outstr));
        ui->output->ensureCursorVisible();
        ui->output->repaint();
    }

    output.str(""); // Clear the stream, because it has already been output
}

void MainWindow::output_text_end()
{
    ui->output->moveCursor(QTextCursor::End);
    ui->output->ensureCursorVisible();
    ui->output->repaint();
}

bool MainWindow::check_stop_pressed() const
{
    QCoreApplication::processEvents();
    return stop_pressed_;
}

void MainWindow::execute_line()
{
    auto line = ui->lineEdit->text();
    clear_input_line();
    ui->output->appendPlainText(QString::fromStdString(MainProgram::PROMPT)+line);

    ui->execute_button->setEnabled(false);
    ui->stop_button->setEnabled(true);
    stop_pressed_ = false;

    ostringstream output;
    bool cont = mainprg_.command_parse_line(line.toStdString(), output);
    ui->lineEdit->clear();
    output_text(output);
    output_text_end();

    ui->stop_button->setEnabled(false);
    ui->execute_button->setEnabled(true);
    stop_pressed_ = false;

    ui->lineEdit->setFocus();

//    if (mainprg_.view_dirty)
//    {
//        update_view();
//        mainprg_.view_dirty = false;
//    }
    update_view();

    if (!cont)
    {
        close();
    }
}

void MainWindow::cmd_selected(int idx)
{
    ui->lineEdit->insert(QString::fromStdString(mainprg_.cmds_[idx].cmd+" "));
    ui->cmd_info_text->setText(QString::fromStdString(mainprg_.cmds_[idx].cmd+" "+mainprg_.cmds_[idx].info));

    ui->lineEdit->setFocus();
}

void MainWindow::number_selected(QString number)
{
    ui->lineEdit->insert(number+" ");

    ui->lineEdit->setFocus();
}

void MainWindow::select_file()
{
    QFileDialog dialog(this, "Select file", "", "Command list (*.txt)");
    dialog.setFileMode(QFileDialog::ExistingFiles);
    if (dialog.exec())
    {
        auto filenames = dialog.selectedFiles();
        for (auto& i : filenames)
        {
            auto filename = QDir("").relativeFilePath(i);
            ui->lineEdit->insert("\""+filename+"\" ");
        }
    }

    ui->lineEdit->setFocus();
}

void MainWindow::clear_input_line()
{
    ui->cmd_info_text->clear();
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();
}

void MainWindow::fit_view()
{
    ui->graphics_view->fitInView(gscene_->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void MainWindow::scene_selection_change()
{
    auto items = gscene_->selectedItems();
    if (!items.empty())
    {
        for (auto i : items)
        {
            auto [beaconid, coord] = i->data(0).value<XroadInfo>();
//            assert(beaconid!=NO_ID && "NO_ID as beacon in graphics view");
            if (!selection_clear_in_progress)
            {
                ostringstream output;
                output << "*click* ";
                if (ui->pick_beacon->isChecked())
                {
                    auto beaconstr = mainprg_.print_beacon(beaconid, output);
                    if (!beaconstr.empty()) { ui->lineEdit->insert(QString::fromStdString(beaconstr+" ")); }
                }
                else
                {
                    auto coordstr = mainprg_.print_coord(coord, output);
                    if (!coordstr.empty()) { ui->lineEdit->insert(QString::fromStdString(coordstr)+" "); }
                }
                output_text(output);
                output_text_end();
            }
            i->setSelected(false);
            selection_clear_in_progress = !selection_clear_in_progress;
        }
    }
}

void MainWindow::clear_selection()
{
    gscene_->clearSelection();
}



// Originally in main.cc
#include <QApplication>

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        QApplication a(argc, argv);
        MainWindow w;
        w.show();

        auto status = a.exec();
        std::cerr << "Program ended normally." << std::endl;
        return status;
    }
    else
    {
        return MainProgram::mainprogram(argc, argv);
    }
}

#else

int main(int argc, char *argv[])
{
    return MainProgram::mainprogram(argc, argv);
}
#endif
