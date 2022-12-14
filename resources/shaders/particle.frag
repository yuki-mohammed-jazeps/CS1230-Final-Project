#version 330 core
out vec4 FragColor;

uniform sampler2D u_part_texture;

//in vec3 fColor;
in vec2 frag_uv;
in float life;
in float coordY;

void main()
{
//    FragColor = texture(u_part_texture, frag_uv);
    vec4 tex = texture(u_part_texture, frag_uv);
    float intensity = 0.299 * tex.x + 0.587 * tex.y + 0.114F * tex.z;
//    if (coordY < 0.01 ) intensity*=coordY*100;
    if (intensity < 0.5) discard;
    vec3 color;

    if (life < 0.1){
        //red
        color = vec3(1.0, 0.3, 0.14);
    }
    else if (life < 0.5){
        //red
        color = vec3(1.0, 0.4, 0.14);
    } else if (life < 3.0){
        //orange
        color = vec3(1.0, 0.66, 0.16);
    } else {
        color = vec3(1.0, 1.0, 0.36);
    }
//    else {
//        color = vec3(1.0, 1.0, 1.0);
//    }


    FragColor = vec4(color, intensity * 0.5);
//    FragColor = vec4(1.0);
}
