#include <qdebug.h>
#include <unordered_map>
#include <cmath>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QString ptcfile = get_ptc_configure();
    m_compounds = load_compounds(ptcfile.toStdString());
    m_methods = load_methods(ptcfile.toStdString());
    ui->setupUi(this);
    this->setFixedSize(1024,768);

    init_drydosing();
    init_volume();
    init_drip_wc();
    init_configure();
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
    ui->drydosing_aquarium_size->setText("100");
    ui->drydosing_concentration->setValidator(new QDoubleValidator(0,1000,5,this));
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

    auto solutions = get_solution_list();
    for(auto& s : solutions) {
        ui->drydosing_solution_list->addItem(s);
    }
    ui->drydosing_solution_list->addItem(tr("Add New"),QVariant(true));
    ui->drydosing_solution_list->setCurrentIndex(solutions.size());
}

void MainWindow::init_drip_wc()
{
    ui->drip_wc_volume_1->setValidator(new QIntValidator(0,100000,this));
    ui->drip_wc_percent->setValidator(new QIntValidator(0,100,this));
    ui->drip_wc_days1->setValidator(new QIntValidator(0,100,this));
    ui->drip_wc_volume2->setValidator(new QIntValidator(0,100000,this));
    ui->drip_wc_each_day_volume->setValidator(new QIntValidator(0,100000,this));

    QStringList header;
    header << tr("Changed percent");
    ui->drip_wc_percentages->setHorizontalHeaderLabels(header);
}

void MainWindow::init_configure()
{
    reset_cfg_edit();
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
                    ui->drydosing_concentration->setText(QString::number(roundf3(e.ppm)));
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
                ui->drydosing_concentration->setText(QString::number(roundf3(e.ppm)));
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
    ui->drydosing_solution_list->setEnabled(checked);
    ui->drydosing_solution_save->setEnabled(checked);

    ui->drydosing_solution_list->setCurrentIndex(-1);
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

        ui->volume_water_volume->setText(QString::number(roundf3(water_vol)));
        ui->volume_substrate->setText(QString::number(roundf3(sub_vol)));
        ui->volume_surface->setText(QString::number(suf_area));
    }
}



static double bad_NRoot(double x,uint32_t N) {
    //利用二分法求解
    if( x <= 0.0 || x >= 1.0 ) return x;
    if(N==1) return x;
    double lower=0;
    double higher=1.0;
    double value=0.5;
	double j;

    for(;;) {
		j = std::pow(value,N) - x;
        qDebug() << "x=" << x << ",N=" << N << ",value=" << value << ",lower=" << lower << ",higher=" << higher << ",j=" << j;
		if( std::abs(j) < 0.0001 ) break;
        if( j > 0 ) {
			higher = value;
		} else {
			lower = value;
		}
		value = ( higher + lower ) / 2.0;
    }
    return value;
}

void MainWindow::on_drip_wc_solution_clicked()
{
    int volume = ui->drip_wc_volume_1->text().toInt();
    int percent = ui->drip_wc_percent->text().toInt();
    int days = ui->drip_wc_days1->text().toInt();

    if( volume < 1 || percent < 1 || percent >= 100 || days < 1 ) return;

    double target = (100 - percent) / 100.0;

    qDebug() << "target: " << target << ", days:" << days;
	double each_day_percent = bad_NRoot(target,days);
	qDebug() << "each_day_percent: " << each_day_percent;
	if( each_day_percent <= 0.0 || each_day_percent >= 1.0 ) return;

    ui->drip_wc_each_volume->setText(QString::number( roundf3(volume * (1.0 - each_day_percent))));
    ui->drip_wc_days2->setText(ui->drip_wc_days1->text());
    ui->drip_wc_target_percent->setText(QString::number( roundf3((1.0 - std::pow(each_day_percent,days)) * 100)) + "%");
}

void MainWindow::on_drip_wc_display_percentages_clicked()
{
    int volume = ui->drip_wc_volume2->text().toInt();
    int each_volume = ui->drip_wc_each_day_volume->text().toInt();
    if( volume < 1 || each_volume >= volume ) return;

    double p = (volume - each_volume) * 1.0 / volume;
    double w = 1.0;
    ui->drip_wc_percentages->setRowCount(100);
    for(int i=0; i < 100; ++i) {
        w = w * p;
        ui->drip_wc_percentages->setItem(i,0,new QTableWidgetItem(QString::number(roundf3(100.0 - (w * 100.0))) + "%"));
    }
}

void MainWindow::reset_cfg_edit()
{
    QString ptcfile = get_ptc_configure();
    QFile file(ptcfile);
    if( file.open(QFile::ReadOnly | QFile::Text)) {
        QString content;
        while( !file.atEnd() ) {
            content.append( file.readLine() );
        }
        ui->cfg_edit->setPlainText(content);
        file.close();
    }
}

void MainWindow::on_cfg_reset_clicked()
{
    reset_cfg_edit();
}

void MainWindow::on_cfg_save_clicked()
{
    QString content = ui->cfg_edit->document()->toPlainText();
    auto cs = load_compounds_from_string(content);
    auto ms = load_methods_from_string(content);
    if( !cs.empty() && !ms.empty() ) {
        m_compounds = std::move(cs);
        m_methods = std::move(ms);

        QFile file(get_ptc_configure());
        if( file.open(QFile::WriteOnly | QFile::Text)) {
            QTextStream writer(&file);
            writer << content;
            writer.flush();
            file.close();
        }
        ui->drydosing_fertilizer->clear();
        for(auto& c : m_compounds) {
            ui->drydosing_fertilizer->addItem(QString::fromStdString(c.formula));
        }
        ui->drydosing_method->clear();
        for(auto& m : m_methods) {
            ui->drydosing_method->addItem(QString::fromStdString(m.name));
        }
    } else {
        QMessageBox::information(this, tr("Warning"),tr("bad configure format"));
    }

}

void MainWindow::on_drydosing_solution_list_currentIndexChanged(int index)
{
    qDebug() << __LINE__ << " " << index;
    do {
        if( !ui->drydosing_is_solution->isChecked() ) break;
        if( index < 0 || index >= ui->drydosing_solution_list->count() ) {
            qDebug() << __LINE__;
            break;
        }
        QString name = ui->drydosing_solution_list->itemText(index);
        QVariant var = ui->drydosing_solution_list->itemData(index);

        if( var.type() == QVariant::Bool ) {
            bool is_add_new = var.toBool();
            if( is_add_new ) { break; }
        }

        std::shared_ptr<solution_t> solution = get_solution(name.toStdString());
        if( solution == nullptr ) { break; }

        ui->drydosing_total_element_ppm->clearContents();
        ui->drydosing_total_fertilizer_weight->clearContents();

        ui->drydosing_aquarium_size->setText(QString::number(solution->tank_l));
        ui->drydosing_container->setText(QString::number(solution->container_ml));
        ui->drydosing_each_dosing->setText(QString::number(solution->dosing_ml));

        ui->drydosing_total_fertilizer_weight->setRowCount(roundf3(solution->fertilizers.size()));
        int row = 0;

        std::unordered_map<std::string,float> total_element_ppm;
        for(auto f : solution->fertilizers) {
            ui->drydosing_total_fertilizer_weight->setItem(row,0,new QTableWidgetItem(QString::fromStdString(f.formula)));
            ui->drydosing_total_fertilizer_weight->setItem(row,1,new QTableWidgetItem(QString::number(roundf3(f.gram))));
            ++row;
            if( solution->tank_l > 0 && solution->container_ml > 0 && solution->dosing_ml > 0 ) {
                auto c = get_compound(f.formula);
                if( c ) {
                    for(int i=0; i < c->elements.size(); ++i) {
                        float mg = f.gram * 1000 * c->get_percent(c->elements[i].formula);
                        float ppm = (mg * solution->dosing_ml / solution->container_ml) / solution->tank_l;

                        total_element_ppm[c->elements[i].formula] += ppm;
                    }
                }
            }
        }
        ui->drydosing_total_element_ppm->setRowCount(total_element_ppm.size());
        row = 0;
        for(auto e : total_element_ppm) {
            ui->drydosing_total_element_ppm->setItem(row,0,new QTableWidgetItem(QString::fromStdString(e.first)));
            ui->drydosing_total_element_ppm->setItem(row,1,new QTableWidgetItem(QString::number(roundf3(e.second))));
            ++row;
        }
    } while(0);
}

void MainWindow::on_drydosing_solution_save_clicked()
{
    do {
        solution_t solution;
        QString name = ui->drydosing_solution_list->currentText();
        QVariant var = ui->drydosing_solution_list->currentData();
        bool add_new = (var.type() == QVariant::Bool && var.toBool());
        if( add_new ) {
            bool is_ok = false;
            name = QInputDialog::getText(nullptr,tr("Solution name"),tr("Please input solution name"),QLineEdit::Normal,tr("name of solution file"),&is_ok);
            if( !is_ok ) break;
        }

        solution.name = name.toStdString();
        solution.tank_l = ui->drydosing_aquarium_size->text().toInt();
        solution.container_ml = ui->drydosing_container->text().toInt();
        solution.dosing_ml = ui->drydosing_each_dosing->text().toFloat();

        for(int row=0; row < ui->drydosing_total_fertilizer_weight->rowCount(); ++row) {
            solution_fertilizer_t f;
            f.formula = ui->drydosing_total_fertilizer_weight->item(row,0)->text().toStdString();
            f.gram = ui->drydosing_total_fertilizer_weight->item(row,1)->text().toFloat();

            solution.fertilizers.push_back(f);
        }
        if( !solution.name.empty() && solution.tank_l > 0 && solution.container_ml > 0 && solution.dosing_ml > 0.0 && !solution.fertilizers.empty() ) {
            save_solution(solution);
        }


        auto solutions = get_solution_list();

        ui->drydosing_solution_list->clear();
        for(auto& s : solutions) {
            ui->drydosing_solution_list->addItem(s);
        }
        ui->drydosing_solution_list->addItem(tr("Add New"),QVariant(true));

        for(int row = 0; row < ui->drydosing_solution_list->count(); ++row) {
            if( ui->drydosing_solution_list->itemText(row).compare(name) == 0 ) {
                ui->drydosing_solution_list->setCurrentIndex(row);
                break;
            }
        }
        //ui->drydosing_solution_list->setCurrentText(name);

    } while(0);
}
