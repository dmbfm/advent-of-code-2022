#![allow(dead_code)]

use std::fs::File;
use std::io::{self, BufRead, Read, Seek, Write};
use std::collections::HashSet;

#[derive(Clone, Copy)]
struct Position {
    x: i32,
    y: i32,
}

#[derive(Clone, Copy, Debug)]
enum Direction {
    R,
    L,
    U,
    D,
}

impl Position {
    fn do_move(self: &mut Self, m: Direction) {
        match m {
            Direction::R => self.x += 1,
            Direction::L => self.x -= 1,
            Direction::U => self.y += 1,
            Direction::D => self.y -= 1,
        }
    }

    fn follow(self: &mut Self, head: Position) {
        let dx = head.x - self.x;
        let dy = head.y - self.y;

        if dx.abs() <= 1 && dy.abs() <= 1 { 
            return; 
        } else if dx.abs() >= 1 && dy.abs() >= 1 {
            self.x += dx.signum();
            self.y += dy.signum();
        } else {
            match (dx, dy) {
                ( 2,  0) => self.x += 1,
                (-2,  0) => self.x -= 1,
                ( 0,  2) => self.y += 1,
                ( 0, -2) => self.y -= 1,
                _ => {},
            }
        }
    }
}

fn parse_move(line: &str) -> Option<(Direction, i32)> {
    let dir = match line.chars().nth(0) {
        Some(val) => val,
        None => return None,
    };
    
    let count: i32 = match line[2..].parse() {
        Ok(val) => val,
        Err(_) => return None,
    };


    match dir {
        'R' => return Some((Direction::R, count)),
        'L' => return Some((Direction::L, count)),
        'U' => return Some((Direction::U, count)),
        'D' => return Some((Direction::D, count)),
         _  => return None,
    }
}

fn ppm(positions: &Vec<Position>, path: &str) {

    let mut min_x: Option<i32> = None;
    let mut min_y: Option<i32> = None;
    let mut max_x: Option<i32> = None;
    let mut max_y: Option<i32> = None;

    for p in positions {
        if let Some(x) = min_x {
            if p.x < x {
                min_x = Some(p.x);
            }
        } else {
            min_x = Some(p.x);
        }
        
        if let Some(x) = max_x {
            if p.x > x {
                max_x = Some(p.x);
            }
        } else {
            max_x = Some(p.x);
        }

         
        if let Some(y) = max_y {
            if p.y > y {
                max_y = Some(p.y);
            }
        } else {
            max_y = Some(p.y);
        }

        if let Some(y) = min_y {
            if p.y < y {
                min_y = Some(p.y);
            }
        } else {
            min_y = Some(p.y);
        }
    }

    println!("x = [{}, {}], y = [{}, {}]", min_x.unwrap(), max_x.unwrap(), min_y.unwrap(), max_y.unwrap());
    
    let width = (max_x.unwrap() - min_x.unwrap() + 1) as usize;
    let height = (max_y.unwrap() - min_y.unwrap() + 1) as usize;

    println!("w = {}, h = {}", width, height);
    
    let mut file = File::create(path).unwrap();
    let mut fb = vec![false; width as usize * height as usize];

    for p in positions {
        let relative_x = (p.x - min_x.unwrap()) as usize;
        let relative_y = (p.y - min_y.unwrap()) as usize;
        fb[relative_y * (width as usize) + relative_x] = true;
    }
    
    write!(file, "P3\n").unwrap();
    write!(file, "{} {}\n", width, height).unwrap();
    write!(file, "255\n").unwrap();

    for y in 0..height {
        for x in 0..width {
            
            let value = if fb[y * width + x] {
                255
            } else { 
                0 
            };
            
            write!(file, "{} {} {} ", value, value, value).unwrap();
        }

        write!(file, "\n").unwrap();
    }
    //write!("P3\n", "h").unwrap();
    
}

fn main() {
    let file = File::open("input").unwrap();
    let mut reader = io::BufReader::new(file);
    
    {
        let mut head = Position { x: 0, y: 0 };
        let mut tail = Position { x: 0, y: 0 };
        let mut positions: Vec<Position> = Vec::new();
        let mut hash = HashSet::new();
        hash.insert("0,0".to_string());

        let lines = reader.by_ref().lines();
        for _line in lines {
            if let Ok(line) = _line {
                let (dir, count) = parse_move(&line).unwrap();

                for _ in 0..count {
                    head.do_move(dir);
                    tail.follow(head);
                    hash.insert(format!("{},{}", tail.x, tail.y));
                    positions.push(tail);
                }
            }
        }

        println!("------ Part 1 ------");
        println!("Final head position: {}, {}", head.x, head.y);
        println!("Final tail position: {}, {}", tail.x, tail.y);
        println!("# visited locations: {}", hash.len());
        ppm(&positions, "part1.ppm");
    }

    reader.seek(io::SeekFrom::Start(0)).unwrap();

    {
        println!("------ Part 2 ------");
        let mut knots = [Position {x: 0, y: 0}; 10];
        let lines = reader.by_ref().lines();
        let mut hash = HashSet::new();
        hash.insert("0,0".to_string());
        let mut positions: Vec<Position> = Vec::new();

        
        for _line in lines {
            let line = _line.unwrap();
            let (dir, count) = parse_move(&line).unwrap();

            for _ in 0..count {
                let head = &mut (knots[0]);
                head.do_move(dir);
                for i in 1..10 {
                    knots[i].follow(knots[i-1]);
                }
                
                if hash.insert(format!("{},{}", knots[9].x, knots[9].y)) {
                    positions.push(knots[9]);
                }
            }
        }

        let head = &knots[0];
        println!("Final head position: {}, {}", head.x, head.y);
        for i in 1..10 {
            println!("Final tail {} position: {}, {}", i,  knots[i].x, knots[i].y);
        }
        println!("# visited locations: {}", hash.len());

        ppm(&positions, "part-2.ppm");
    }
}
