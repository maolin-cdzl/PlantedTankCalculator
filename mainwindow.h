#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "elements.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_drydosing_fertilizer_currentIndexChanged(const QString &arg1);

    void on_drydosing_method_currentIndexChanged(const QString &arg1);

    void on_drydosing_element_currentIndexChanged(const QString &arg1);

    void on_drydosing_calculate_clicked();

    void on_drydosing_is_drydosing_toggled(bool checked);

    void on_drydosing_is_solution_toggled(bool checked);

    void on_drydosing_add_to_total_clicked();

    void on_drydosing_clear_clicked();

    void on_drydosing_container_textChanged(const QString &arg1);

    void on_drydosing_each_dosing_textChanged(const QString &arg1);

    void on_volume_calculate_clicked();

    void on_drip_wc_solution_clicked();

    void on_drip_wc_display_percentages_clicked();

    void on_cfg_reset_clicked();

    void on_cfg_save_clicked();

    void on_drydosing_solution_list_currentIndexChanged(int index);

    void on_drydosing_solution_save_clicked();

private:
    const compound_t* get_compound(const std::string& formula) const;
    const dosing_method_t* get_method(const std::string& method) const;
    void init_drydosing();
    void init_volume();
    void init_drip_wc();
    void init_configure();
    void clear_drydosing();
    void update_concertration();
    void reset_cfg_edit();
private:
    Ui::MainWindow *ui;
    std::vector<compound_t>         m_compounds;
    std::vector<dosing_method_t>    m_methods;
};

#endif // MAINWINDOW_H
