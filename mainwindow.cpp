#include <qdebug.h>
#include <unordered_map>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    m_compounds = load_compounds("ptc.cfg");
    m_methods = load_methods("ptc.cfg");
    ui->setupUi(this);
    init_drydosing();
    init_volume();
}

MainWindow::~MainWindow()
{
    delete ui;
}

const compound_t* MainWindow::get_compound(const std::string& formula) const {
    for(auto& c : m_compounds) {
        if( c.formula.compare(formula) == 0 ) {
            return &c;
        }
    }
    return nullptr;
}

const dosing_method_t* MainWindow::get_method(const std::string& method) const {
    for(auto& m : m_methods) {
        if( m.name.compare(method) == 0 ) {
            return &m;
        }
    }
    return nullptr;
}

void MainWindow::init_drydosing()
{
    ui->drydosing_aquarium_size->setValidator(new QIntValidator(0,100000,this));
    ui->drydosing_aquarium_size->setText("10");
    ui->drydosing_concentration->setValidator(new QDoubleValidator(0,1000,2,this));
    ui->drydosing_container->setValidator(new QIntValidator(0,100000,this));
    ui->drydosing_each_dosing->setValidator(new QIntValidator(0,100000,this));

    for(auto& c : m_compounds) {
        ui->drydosing_fertilizer->addItem(QString::fromStdString(c.formula));
    }
    for(auto& m : m_methods) {
        ui->drydosing_method->addItem(QString::fromStdString(m.name));
    }

    QStringList header;
    header << tr("Element") << tr("PPM");
    ui->drydosing_element_ppm->setHorizontalHeaderLabels(header);
    ui->drydosing_total_element_ppm->setHorizontalHeaderLabels(header);
    header.clear();
    header << tr("Fertilizer") << tr("Gram");
    ui->drydosing_fertilizer_weight->setHorizontalHeaderLabels(header);
    ui->drydosing_total_fertilizer_weight->setHorizontalHeaderLabels(header);

}

void MainWindow::clear_drydosing() {
    ui->drydosing_element_ppm->clearContents();
    ui->drydosing_element_ppm->setRowCount(0);
    ui->drydosing_fertilizer_weight->clearContents();
    ui->drydosing_fertilizer_weight->setRowCount(0);
    ui->drydosing_total_element_ppm->clearContents();
    ui->drydosing_total_element_ppm->setRowCount(0);
    ui->drydosing_total_fertilizer_weight->clearContents();
    ui->drydosing_total_fertilizer_weight->setRowCount(0);
}

void MainWindow::init_volume() {
    ui->volume_length->setValidator(new QIntValidator(0,1200,this));
    ui->volume_width->setValidator(new QIntValidator(0,1200,this));
    ui->volume_high->setValidator(new QIntValidator(0,1200,this));
    ui->volume_sub_thickness->setValidator(new QIntValidator(0,100,this));
    ui->volume_surface_to_top->setValidator(new QIntValidator(0,100,this));
}

void MainWindow::on_drydosing_fertilizer_currentIndexChanged(const QString &arg1)
{
    ui->drydosing_element->clear();
    auto* c = get_compound(arg1.toStdString());
    if( c ) {
        for(auto& e : c->elements) {
            if( !e.ignore ) {
                ui->drydosing_element->addItem(QString::fromStdString(e.formula));
            }
        }
    }
}

void MainWindow::on_drydosing_method_currentIndexChanged(const QString &arg1)
{
    auto* m = get_method(arg1.toStdString());
    if( m ) {
        auto en = ui->drydosing_element->currentText().toStdString();
        if( ! en.empty() ) {
            for(auto& e : m->elements) {
                if( e.formula.compare(en) == 0 ) {
                    ui->drydosing_concentration->setText(QString::number(e.ppm));
                    return;
                }
            }
        } else {
            qDebug() << "get element failed";
        }
    } else {
        qDebug() << "get_method faild, " << arg1;
    }

    ui->drydosing_concentration->setText("0");
}

void MainWindow::on_drydosing_element_currentIndexChanged(const QString &arg1)
{
    auto en = arg1.toStdString();
    auto* m = get_method(ui->drydosing_method->currentText().toStdString());

    if( m && !en.empty() ) {
        for(auto& e : m->elements) {
            if( e.formula.compare(en) == 0 ) {
                ui->drydosing_concentration->setText(QString::number(e.ppm));
                return;
            }
        }
    }
    ui->drydosing_concentration->setText("0");
}

void MainWindow::on_drydosing_calculate_clicked()
{
    ui->drydosing_element_ppm->clearContents();
    ui->drydosing_element_ppm->setRowCount(0);

    auto* c = get_compound(ui->drydosing_fertilizer->currentText().toStdString());
    const int size  = ui->drydosing_aquarium_size->text().toInt();
    auto en = ui->drydosing_element->currentText().toStdString();
    float ppm = ui->drydosing_concentration->text().toFloat();


    if( c && size > 0 && !en.empty() && ppm > 0.0f ) {
        float percent = c->get_percent(en);
        const float element_mg = size * ppm;

        if( percent > 0.0f && element_mg > 0.0f ) {
            float compount_mg = element_mg / percent;
            float fertilizer_mg = compount_mg;
            if( ui->drydosing_is_solution->isChecked() ) {
                int container = ui->drydosing_container->text().toInt();
                int each_dosing = ui->drydosing_each_dosing->text().toInt();

                if( container <= 0 || each_dosing <= 0 || container < each_dosing ) {
                    return;
                }
                fertilizer_mg = fertilizer_mg * container / each_dosing;
            }
            qDebug() << "percent " << percent << ", element mg " << element_mg << ", compount mg " << compount_mg;

            ui->drydosing_element_ppm->setRowCount(c->elements.size());
            for(int i=0; i < c->elements.size(); ++i) {
                ui->drydosing_element_ppm->setItem(i,0,new QTableWidgetItem(QString::fromStdString(c->elements[i].formula)));

                percent = c->get_percent(c->elements[i].formula);
                qDebug() << c->elements[i].formula.c_str() << " percent " << percent;
                ui->drydosing_element_ppm->setItem(i,1,new QTableWidgetItem(QString::number(compount_mg * percent / size)));
            }

            ui->drydosing_fertilizer_weight->setRowCount(1);
            ui->drydosing_fertilizer_weight->setItem(0,0,new QTableWidgetItem(QString::fromStdString(c->formula)));
            ui->drydosing_fertilizer_weight->setItem(0,1,new QTableWidgetItem(QString::number(fertilizer_mg / 1000)));
        }
    }
}


void MainWindow::on_drydosing_is_drydosing_toggled(bool checked)
{
    qDebug() << "on_drydosing_is_drydosing_toggled " << checked;
    clear_drydosing();
}

void MainWindow::on_drydosing_is_solution_toggled(bool checked)
{
    qDebug() << "on_drydosing_is_solution_toggled " << checked;
    clear_drydosing();

    ui->drydosing_container->setEnabled(checked);
    ui->drydosing_each_dosing->setEnabled(checked);
}

void MainWindow::on_drydosing_add_to_total_clicked()
{
    std::unordered_map<std::string,float>       element_ppm,total_element_ppm,fertilizer,total_fertilizer;

    for(int rc = 0; rc < ui->drydosing_element_ppm->rowCount(); ++rc) {
        auto c = ui->drydosing_element_ppm->item(rc,0)->text().toStdString();
        float ppm = ui->drydosing_element_ppm->item(rc,1)->text().toFloat();
        element_ppm[c] = ppm;
    }
    for(int rc = 0; rc < ui->drydosing_total_element_ppm->rowCount(); ++rc) {
        auto c = ui->drydosing_total_element_ppm->item(rc,0)->text().toStdString();
        float ppm = ui->drydosing_total_element_ppm->item(rc,1)->text().toFloat();
        total_element_ppm[c] = ppm;
    }

    for(auto e : element_ppm) {
        total_element_ppm[e.first] += e.second;
    }

    ui->drydosing_total_element_ppm->clearContents();
    ui->drydosing_total_element_ppm->setRowCount(0);
    if( ! total_element_ppm.empty() ) {
        ui->drydosing_total_element_ppm->setRowCount(total_element_ppm.size());
        int row = 0;
        for(auto e : total_element_ppm) {
            ui->drydosing_total_element_ppm->setItem(row,0,new QTableWidgetItem(QString::fromStdString(e.first)));
            ui->drydosing_total_element_ppm->setItem(row,1,new QTableWidgetItem(QString::number(e.second)));
            ++row;
        }
    }

    for(int rc = 0; rc < ui->drydosing_fertilizer_weight->rowCount(); ++rc) {
        auto c = ui->drydosing_fertilizer_weight->item(rc,0)->text().toStdString();
        float gram = ui->drydosing_fertilizer_weight->item(rc,1)->text().toFloat();
        fertilizer[c] = gram;
    }
    for(int rc = 0; rc < ui->drydosing_total_fertilizer_weight->rowCount(); ++rc) {
        auto c = ui->drydosing_total_fertilizer_weight->item(rc,0)->text().toStdString();
        float gram = ui->drydosing_total_fertilizer_weight->item(rc,1)->text().toFloat();
        total_fertilizer[c] = gram;
    }
    for(auto f : fertilizer) {
        total_fertilizer[f.first] += f.second;
    }
    ui->drydosing_total_fertilizer_weight->clearContents();
    ui->drydosing_total_fertilizer_weight->setRowCount(0);
    if( ! total_fertilizer.empty() ) {
        ui->drydosing_total_fertilizer_weight->setRowCount(total_fertilizer.size());
        int row = 0;
        for(auto f : total_fertilizer) {
            ui->drydosing_total_fertilizer_weight->setItem(row,0,new QTableWidgetItem(QString::fromStdString(f.first)));
            ui->drydosing_total_fertilizer_weight->setItem(row,1,new QTableWidgetItem(QString::number(f.second)));
            ++row;
        }
    }
}

void MainWindow::on_drydosing_clear_clicked()
{
    clear_drydosing();
}

void MainWindow::on_drydosing_container_textChanged(const QString &arg1)
{
    clear_drydosing();
}

void MainWindow::on_drydosing_each_dosing_textChanged(const QString &arg1)
{
    clear_drydosing();
}

void MainWindow::on_volume_calculate_clicked()
{
    int length = ui->volume_length->text().toInt();
    int width = ui->volume_width->text().toInt();
    int high = ui->volume_high->text().toInt();
    int sub = ui->volume_sub_thickness->text().toInt();
    int suf = ui->volume_surface_to_top->text().toInt();

    if( length > 0 && width >0 && high > 0 && high > (sub + suf) ) {
        float water_vol = length * width * (high - sub - suf ) / 1000.0;
        float sub_vol = length * width * sub / 1000.0;
        int suf_area = length * width;

        ui->volume_water_volume->setText(QString::number(water_vol));
        ui->volume_substrate->setText(QString::number(sub_vol));
        ui->volume_surface->setText(QString::number(suf_area));
    }
}
