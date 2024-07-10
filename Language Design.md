# Langauge Design

## Goals

* Create a simple brace style language with minimal features
* Extremely fast compilation and error reporting. (< 1 sec for 250000 lines)
* A simple byte code format for compile time and possible interface to editor
* C code generation (maximum portability)

## Features

* Syntax based features
  * Simple generics, AST introspection allows user to see types for use cases instantly
  * Expression based
  * Pattern matching
  * Traits kind of like Rust, but simpler

* AST based features
  * Error nodes with solutions

* Generics
  * Monomorphic generics. Monomorphization generates code/byte code for each type
  essentially copy/paste with type checking

* Enums
  * Basically tagged unions
  * Warnings instead of errors when you haven't covered all variants in a match

* SIMD
  * Should be a base type, and offer better coverage. 
  * Hopefully can be written as scalar but with the correct types and work well
  * Generate code to process an array of items, say 4 at a time and auto gen the case to handle the remainder.


## Example Code

``` Rust
// normal function
fn do_work(times: u32) {
    
}

// varatic function
fn print_ln(format_string: str, ...) {

}

// bit fields - not sure I like this
struct Bits {
    lsb_flag: u1;
    a_few_bits: u3;
}

// simple structure with generic type
struct Vec3<T> {
    // favor single line fields with same type...
    x, y, z: T;
    // constant defined in the structure will have the call to Vec3::new() run at
    // compile time, so all variables passed must be constant, i.e. we couldn't
    // pass Vec3::new(rand()) as rand is not constant and needs to run at runtime
    const Zero = Vec3::new(0);
}

// implemetation for above type
impl Vec3<T> {
    // constants can be defined inside the impl or inside the structure definition,
    // whatever feels better
    const One = Vec3::new(1);

    // if you declare self as the first type, all fields are directly available
    fn len(self) -> T {
        // if the given type T doesn't have a compatible sqrt function we will 
        // provide an error, if there is a compatible function but it causes a
        // loss in precision then you get a warning
        return sqrt(x * x + y * y + z * z);
    }

    // method without self is an associated function called like Vec3<f32>::new()
    // or Vec3::new(1.0) with type inference
    fn new(v: T) -> Vec3<T> {
        // return is manditory, unlike rust
        return Vec3 {
            x: v,
            y: v,
            z: v,
        }
    }
}

// traits define methods and variables that a stucture must implement to be compliant
trait Printable {
    // here we define a variable that the trait itself defines, so no redefiition is
    // needed, but this variable will exist in each structure object that implements
    // this trait
    font_size: f32 = 14;
    // here is a method that must be defined by implmentors
    fn print();
    // this method has a default implementation, so if no impl is defined then this 
    // runs, if it is implmented then that version will be called, again this is 
    // monomorphic under the hood, so this method will be generated for each type it's
    // implemented for, even the default version, this means code side penalty for
    // zero run time overhead
    fn print_ln() {
        print();
        print("\n");
    }
}

impl Printable for Vec3<T> {
    fn print() {
        // code to print the vector
    }
}

// trait objects do not exist the way they do in rust
fn accept_trait(obj: Printable) {
    // obj can be any object that implements Printable, under the hood we again use
    // monomorphic code generation, this is not dynamic in nature, there is a finite
    // set of places vs possible objects that can have this called, it is however 
    // a double pointer, because each object passed can have its fields and methods 
    // defined at defferent places in memory
}
```