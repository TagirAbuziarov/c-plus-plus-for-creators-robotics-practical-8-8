#include <chrono>
#include <complex>
#include <math.h>

#include <QApplication>
#include <QDir>
#include <QString>
#include <QImage>
#include <QPainter>
#include <QDebug>

#include "snake.h"

const QSize Snake::cell_size_{SNAKE_CELL_SIZE, SNAKE_CELL_SIZE};

// Наследование от базового класса объектов ui
Snake::Snake(QWidget * parent) 
    : QWidget(parent), 
    is_apple_eaten_(false),
    is_game_over_(false)
{
    // Загрузка изображений змейки и яблока
    this->loadImages();
    
    /* 
    Создает змейку, случайным образом располагает яблоко на игровом поле и 
    запускает игру 
    */
    this->initGame();

    // Фиксация размера окна
    this->setFixedSize(SNAKE_WINDOW_WIDTH, SNAKE_WINDOW_HEIGHT);
    
}

    
void Snake::loadImages()
{
    QDir dir {QDir::current()};
    dir.cd(QString("../") + SNAKE_IMAGE_FOLDER);
    QString images_directory_path {dir.absolutePath() + "/"};

    // Все используемые изображения масштабируются в квадратные

    this->apple_image_.load(
        images_directory_path + SNAKE_APPLE_IMAGE_FILE_NAME);
    this->apple_image_ = this->apple_image_.scaled(this->cell_size_);

    this->body_image_.load(
        images_directory_path + SNAKE_BODY_IMAGE_FILE_NAME);
    this->body_image_ = this->body_image_.scaled(this->cell_size_);

    this->head_image_.load(
        images_directory_path + SNAKE_HEAD_IMAGE_FILE_NAME);
    this->head_image_ = this->head_image_.scaled(this->cell_size_);

    this->game_over_image_.load(
        images_directory_path + SNAKE_GAME_OVER_IMAGE_FILE_NAME);
    int image_size {std::min(SNAKE_WINDOW_WIDTH, SNAKE_WINDOW_HEIGHT)};
    QSize image_qsize {image_size, image_size};
    this->game_over_image_ = this->game_over_image_.scaled(image_qsize);
}

void Snake::initGame()
{
    // Сброс флага окончания игры
    this->is_game_over_ = false;

    /*
    Использование текущего времени в качестве seed для генератора случайных 
    чисел 
    */
    std::srand(std::time(nullptr));

    // Расположение змейки
    locateSnake();

    // Случайное расположение яблока
    locateApple();

    // Создание таймера игрового цикла
    if (!this->timerId_)
    {
        this->timerId_ = startTimer(std::chrono::milliseconds(SNAKE_UPDATE_PERIOD_MS));
    }
}

void Snake::locateSnake()
{
    // Удаление всех элементов змейки, если они были
    this->snake_bodypart_coordinates_.clear();
    this->snake_bodypart_collisions_.clear();

    for(int i{0}; i < SNAKE_INIT_LENGTH; i++)
    {
        // Заполнение координат звеньев змейки
        QPoint snake_bodypart_coordinate(
            SNAKE_INIT_HEAD_CELL_X * SNAKE_CELL_SIZE, 
            (SNAKE_INIT_HEAD_CELL_Y + i) * SNAKE_CELL_SIZE);
        this->snake_bodypart_coordinates_.append(snake_bodypart_coordinate);

        /* 
        Создание фигур для проверки змейки на предмет коллизий с собственным 
        телом
        */
        QRect snake_bodypart_collision_rect(
            snake_bodypart_coordinate, 
            this->cell_size_
        );
        QRegion snake_bodypart_collision_region(
            snake_bodypart_collision_rect,
            QRegion::Ellipse
        );
        this->snake_bodypart_collisions_.append(
            snake_bodypart_collision_region);
    }

    // Установка направления "вверх" по умолчанию
    this->direction_ = -M_PI_2;
    this->direction_prev_ = this->direction_;
}

void Snake::locateApple()
{
    do
    {
        int apple_x_cell =
            std::rand() / ((RAND_MAX + 1u) / (SNAKE_H_CELLS));
        int apple_y_cell =
            std::rand() / ((RAND_MAX + 1u) / (SNAKE_V_CELLS));

        this->apple_coordinates_ = QPoint(
            apple_x_cell * SNAKE_CELL_SIZE,
            apple_y_cell * SNAKE_CELL_SIZE);

        QRect apple_collision_rect(
            this->apple_coordinates_,
            this->cell_size_);

        this->apple_collision_ = QRegion(
            apple_collision_rect,
            QRegion::Ellipse);

    } while (this->isAppleCollideSnakeBody());
}

void Snake::keyPressEvent(QKeyEvent *event)
{
    // Новое направление движения змейки
    double new_direction {this->direction_};

    // Флаг нажатия кнопки управления
    bool control_key_pressed {false};

    // Обработка нажатия кнопок
    switch (event->key())
    {
    // Продолжение игры по кнопке Esc
    case Qt::Key_Escape:
        if (this->is_game_over_)
        {
            this->initGame();
        }
        break;

    // Обработка нажатия кнопок управления змейкой
    case Qt::Key_Up:
        new_direction = -M_PI_2;
        control_key_pressed = true;
        break;

    case Qt::Key_Down:
        new_direction = M_PI_2;
        control_key_pressed = true;
        break;

    case Qt::Key_Left:
        new_direction = M_PI;
        control_key_pressed = true;
        break;

    case Qt::Key_Right:
        new_direction = 0.;
        control_key_pressed = true;
        break;
    }

    // Запрет движения змейки в обратном направлении или если игра окончена
    if (abs(new_direction - this->direction_prev_) != M_PI
        && control_key_pressed 
        && !this->is_game_over_)
    {
        // Сохранение введенного направления
        this->direction_ = new_direction;

        // Перемещение змейки, не дожидаясь таймера
        if (this->timerId_)
        {
            this->killTimer(this->timerId_);
            this->timerId_ = 0;
        }
        this->updateGame();

        // Обновить таймер
        if (!this->timerId_ && !this->is_game_over_)
        {
            this->timerId_ = startTimer(
                std::chrono::milliseconds(SNAKE_UPDATE_PERIOD_MS));
        }
    }
}

void Snake::timerEvent(QTimerEvent *e)
{
    Q_UNUSED(e);
    this->updateGame();
}

void Snake::updateGame()
{
    // Переместить змейку
    this->moveSnake();

    /*
    Проверка на столкновение змейки с другими объектами и вместимость
    змейки в игровое поле 
    */
    if (this->checkCollision() || this->isSnakeTooBig())
    {
        this->onEndGame();

        if (this->isSnakeTooBig())
        {
            qInfo() << "win";
        }
    }

    // Перерисовать игровое поле
    QWidget::repaint();
}

void Snake::moveSnake()
{
    // Базовое перемещение головы змейки - на величину SNAKE_CELL_SIZE
    std::complex<double> unit_vector_x((double)SNAKE_CELL_SIZE, 0.);

    /* 
    Вращение базового перемещения головы змейки на угол direction_ [рад]
    с использованием библиотеки комплексных чисел
    */
    std::complex<double> delta{
        unit_vector_x * std::polar(1., this->direction_)};

    // Приведение перемещения к типу QPoint с округлением
    QPoint delta_point(
        static_cast<int>(round(delta.real())), 
        static_cast<int>(round(delta.imag()))
    );

    // Вычисление новых абсолютных координат головы змейки
    QPoint new_point = this->snake_bodypart_coordinates_.at(0) + delta_point;

    // Смещение координат ячеек змейки
    this->snake_bodypart_coordinates_.push_front(new_point);
    if (!this->is_apple_eaten_)
    {
        /* 
        Если съедено яблоко, последний элемент не удаляется: тело удлиннилось
        */
        this->snake_bodypart_coordinates_.pop_back();
    }

    // Определение новой области коллизии для головы змейки
    QRect snake_new_head_collisions_rect(
        new_point,
        this->cell_size_
    );
    QRegion snake_new_head_collisions_region(
        snake_new_head_collisions_rect,
        QRegion::Ellipse
    );

    // Обновление фигур коллизий змейки
    this->snake_bodypart_collisions_.push_front(
        snake_new_head_collisions_region);
    if (!this->is_apple_eaten_)
    {
        /* 
        Если съедено яблоко, последний элемент не удаляется: тело удлиннилось
        */
        this->snake_bodypart_collisions_.pop_back();
    }
    
    // Сброс флага съеденного яблока, если он был установлен
    this->is_apple_eaten_ = false;

    // Сохранение направления движения на этой итерации
    this->direction_prev_ = this->direction_;

}

bool Snake::checkCollision()
{
    if (this->isSnakeCollideWall())
    {
        return true;
    }
    
    if (this->isSnakeSelfCollide())
    {
        return true;
    }

    if (this->isAppleCollideSnakeHead())
    {
        this->is_apple_eaten_ = true;
        this->locateApple();
    }
    
    return false;
}

bool Snake::isSnakeCollideWall()
{
    // Проверка на коллизию со стеной
    int x{this->snake_bodypart_coordinates_.at(0).x()};
    int y{this->snake_bodypart_coordinates_.at(0).y()};
    return x > SNAKE_WINDOW_WIDTH - SNAKE_CELL_SIZE  || x < 0 ||
        y > SNAKE_WINDOW_HEIGHT - SNAKE_CELL_SIZE || y < 0;  
}

bool Snake::isSnakeSelfCollide()
{
    // Проверка на коллизию змеи с собственным телом
    QRegion head = this->snake_bodypart_collisions_.at(0);
    for (int i=2; i < this->snake_bodypart_collisions_.size(); i++)
    {
        if (head.intersects(this->snake_bodypart_collisions_.at(i)))
        {
            return true;
        }
    }
    return false;
}

bool Snake::isAppleCollideSnakeHead()
{
    // Проверка на коллизию яблока с головой змеи
    return this->snake_bodypart_collisions_.at(0)
                .intersects(this->apple_collision_);
}

bool Snake::isAppleCollideSnakeBody()
{
    // Проверка на коллизию яблока с телом змеи, включая голову
    for (auto shape: this->snake_bodypart_collisions_)
    {
        if (this->apple_collision_.intersects(shape))
        {
            return true;
        }
    }
    return false;
}

bool Snake::isSnakeTooBig()
{
    return this->snake_bodypart_coordinates_.size() 
        >= SNAKE_H_CELLS * SNAKE_V_CELLS - 1u;
}

void Snake::onEndGame()
{
    this->is_game_over_ = true;
    if (this->timerId_)
    {
        this->killTimer(this->timerId_);
        this->timerId_ = 0;
    }
}

void Snake::paintEvent(QPaintEvent * e)
{
    Q_UNUSED(e);

    // Отрисовка фона
    QPainter painter(this);
    painter.fillRect(
        0, 0, SNAKE_WINDOW_WIDTH, SNAKE_WINDOW_HEIGHT, Qt::GlobalColor::black);

    // Отрисовка яблока
    painter.drawImage(this->apple_coordinates_, this->apple_image_);

    // Отрисовка тела змейки
    for(int i{1}; i < this->snake_bodypart_coordinates_.size(); i++)
    {
        painter.drawImage(this->snake_bodypart_coordinates_.at(i), this->body_image_);
    }

    // Отрисовка головы змейки
    painter.drawImage(snake_bodypart_coordinates_.first(), this->head_image_);

    // Отрисовка надписи "Game over" в случае конца игры
    if (this->is_game_over_)
    {
        int image_size {this->game_over_image_.height()};
        int x_pos {(SNAKE_WINDOW_WIDTH - image_size)/2};
        int y_pos {(SNAKE_WINDOW_HEIGHT - image_size)/2};
        painter.drawImage(x_pos, y_pos, this->game_over_image_);
    }

}




